#include "services/PeopleService.h"

#include <string>

#include <drogon/drogon.h>

#include "services/ScheduleService.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Карточка преподавателя/сотрудника — SELECT-часть поиска и профиля.
// Роль в каталоге: staff-карточка опознаётся конвенцией Р-14
// (teacher_profiles.email = users.login || '@example.edu' при
// users.role='staff') — колонки роли в DDL 0006 нет (02 §4.1 — гайд §9а
// «точно по спеке»). is_staff через LEFT JOIN — 0/1.
const std::string kTeacherSelect =
    "SELECT tp.id, tp.fio, tp.title, tp.department, tp.email, tp.office, "
    "(su.id IS NOT NULL) AS is_staff "
    "FROM teacher_profiles tp LEFT JOIN users su "
    "ON su.role = 'staff' AND tp.email = su.login || '@example.edu' ";

// Выборка студентов: JOIN groups — название группы (публичное «группа»),
// JOIN profiles — ФИО/факультет. Факультет-фильтр и q — через «пустая
// строка = фильтра нет» (фиксированное число параметров, урок Э5: порядок
// $N в тексте = порядку биндинга, поэтому каждое значение — свой $N).
const std::string kStudentSearch =
    "SELECT u.id, u.login, p.fio, p.faculty, g.name AS group_name "
    "FROM users u JOIN profiles p ON p.user_id = u.id "
    "JOIN groups g ON g.id = p.group_id "
    "WHERE u.role = 'student' "
    "AND ($1 = '' OR p.faculty = $1) "
    "AND ($2 = '' OR p.fio LIKE $2 ESCAPE '\\') "
    "ORDER BY u.id";

// Полный профиль студента (свой) — все поля /v1/me + почта-конвенция.
const std::string kStudentFull =
    "SELECT u.id, u.login, u.role, p.fio, p.faculty, p.program, p.course, "
    "p.group_id, p.student_card_no, g.name AS group_name "
    "FROM users u JOIN profiles p ON p.user_id = u.id "
    "JOIN groups g ON g.id = p.group_id "
    "WHERE u.id = $1 AND u.role = 'student'";

// Цель расписания студента: группа по users.id (только студент).
const std::string kStudentTarget =
    "SELECT u.id, u.role, p.group_id FROM users u "
    "JOIN profiles p ON p.user_id = u.id "
    "WHERE u.id = $1 AND u.role = 'student'";

// Всё время (расписание целиком — демо-неделя фикстур): лексикографическое
// сравнение «…Z»-строк корректно для фиксированного формата ISO UTC.
constexpr const char* kAllFrom = "1900-01-01T00:00:00Z";
constexpr const char* kAllTo = "3000-01-01T00:00:00Z";

// LIKE-паттерн с экранированием спецсимволов: пользовательский q не может
// ни сломать SQL (параметр), ни расширить поиск через %/_ (ESCAPE '\').
std::string likePattern(const std::string& raw)
{
    std::string out = "%";
    for (const char c : raw)
    {
        if (c == '%' || c == '_' || c == '\\')
            out += '\\';
        out += c;
    }
    out += "%";
    return out;
}

Json::Value optString(const Row& r, const char* col)
{
    if (r[col].isNull())
        return Json::Value();
    return Json::Value(r[col].as<std::string>());
}

Json::Value teacherJson(const Row& r)
{
    Json::Value p(Json::objectValue);
    // id в people-пространстве (Р-15): teacher_profiles.id + kPersonIdOffset.
    p["id"] = r["id"].as<int64_t>() + PeopleService::kPersonIdOffset;
    p["role"] = r["is_staff"].as<int>() != 0 ? "staff" : "teacher";
    p["fio"] = r["fio"].as<std::string>();
    p["title"] = optString(r, "title");
    p["department"] = optString(r, "department");
    p["email"] = r["email"].as<std::string>();
    p["office"] = optString(r, "office");
    return p;
}

Json::Value studentRowJson(const Row& r)
{
    Json::Value p(Json::objectValue);
    p["id"] = r["id"].as<int64_t>();
    p["role"] = "student";
    p["fio"] = r["fio"].as<std::string>();
    p["group"] = r["group_name"].as<std::string>();
    p["email"] = PeopleService::studentEmail(r["login"].as<std::string>());
    return p;
}

}  // namespace

void PeopleService::loadRequester(DbClientPtr db,
                                  int64_t userId,
                                  std::function<void(Status, Requester)> cb)
{
    *db << "SELECT u.role, p.faculty, p.group_id FROM users u "
           "LEFT JOIN profiles p ON p.user_id = u.id WHERE u.id = $1"
        << userId >>
        [cb, userId](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                // Сессия указывает на несуществующего юзера — fail-closed.
                LOG_ERROR << "PeopleService::loadRequester: user " << userId
                          << " не найден";
                cb(Status::Error, Requester{});
                return;
            }
            const Row& r = rows[0];
            Requester req;
            req.userId = userId;
            req.role = r["role"].as<std::string>();
            if (!r["faculty"].isNull())
                req.faculty = r["faculty"].as<std::string>();
            if (!r["group_id"].isNull())
                req.groupId = r["group_id"].as<int64_t>();
            cb(Status::Ok, std::move(req));
        } >>
        [cb](const DrogonDbException& e) mutable {
            LOG_ERROR << "PeopleService::loadRequester: " << e.base().what();
            cb(Status::Error, Requester{});
        };
}

void PeopleService::search(DbClientPtr db,
                           const Requester& requester,
                           const SearchQuery& q,
                           ResultCb cb)
{
    // Словарь role (02 §5.2): teacher | student | staff; отсутствие/пустая
    // строка — «все». Иное значение — BadRequest (400 контроллера).
    if (q.role && *q.role != "teacher" && *q.role != "student" &&
        *q.role != "staff")
    {
        cb(Result{Status::BadRequest, Json::Value(),
                  "role — ожидается teacher, student или staff"});
        return;
    }

    const bool wantTeachers = !q.role || *q.role == "teacher";
    const bool wantStaff = !q.role || *q.role == "staff";
    const bool wantStudents = !q.role || *q.role == "student";

    // Студент-запрашивающий ограничен СВОИМ факультетом (факультет — из БД
    // по сессии, не из query — 02 §5.2 «студенты — своего факультета»);
    // staff видит всех, плюс опциональный фильтр ?faculty= из query
    // (применяется только к студентам).
    std::string facultyArg;
    if (requester.role == "student")
    {
        if (requester.faculty)
            facultyArg = *requester.faculty;
    }
    else if (q.faculty)
    {
        facultyArg = *q.faculty;
    }
    const std::string qArg = q.q ? likePattern(*q.q) : "";

    // Накопитель переживает цепочку шагов teachers → staff → students
    // (shared_ptr: каждая лямбда — копия, цепочка линейна, стреляет один
    // колбэк).
    struct State
    {
        Json::Value people{Json::arrayValue};
        bool failed = false;
    };
    auto state = std::make_shared<State>();

    auto finish = [state, cb]() mutable {
        if (state->failed)
        {
            cb(Result{Status::Error, Json::Value(), ""});  // fail-closed 500
            return;
        }
        Json::Value body;
        body["people"] = state->people;
        cb(Result{Status::Ok, std::move(body), ""});
    };

    // --- шаг 3: студенты (свой факультет у студента; staff — все) ---------
    auto runStudents = [db, state, finish, wantStudents, facultyArg,
                        qArg]() mutable {
        if (state->failed || !wantStudents)
        {
            finish();
            return;
        }
        *db << kStudentSearch << facultyArg << qArg >>
            [state, finish](const drogon::orm::Result& rows) mutable {
                for (const auto& r : rows)
                    state->people.append(studentRowJson(r));
                finish();
            } >>
            [state, finish](const DrogonDbException& e) mutable {
                LOG_ERROR << "PeopleService::search(students): "
                          << e.base().what();
                state->failed = true;  // fail-closed: без частичного ответа
                finish();
            };
    };

    // --- шаг 2: сотрудники-карточки (Р-14, все; q по fio/title/department)
    auto runStaff = [db, state, finish, wantStaff, qArg,
                     runStudents]() mutable {
        if (state->failed || !wantStaff)
        {
            runStudents();
            return;
        }
        const std::string sql =
            "SELECT tp.id, tp.fio, tp.title, tp.department, tp.email, "
            "tp.office, 1 AS is_staff "
            "FROM teacher_profiles tp JOIN users su "
            "ON su.role = 'staff' AND tp.email = su.login || '@example.edu' "
            "WHERE ($1 = '' OR tp.fio LIKE $1 ESCAPE '\\' "
            "OR tp.title LIKE $1 ESCAPE '\\' "
            "OR tp.department LIKE $1 ESCAPE '\\') ORDER BY tp.id";
        *db << sql << qArg >>
            [state, finish, runStudents](const drogon::orm::Result& rows) mutable {
                for (const auto& r : rows)
                    state->people.append(teacherJson(r));
                runStudents();
            } >>
            [state, finish, runStudents](const DrogonDbException& e) mutable {
                LOG_ERROR << "PeopleService::search(staff): "
                          << e.base().what();
                state->failed = true;
                runStudents();  // следующий шаг увидит failed → finish
            };
    };

    // --- шаг 1: преподаватели (все; q по fio/title/department) ------------
    auto runTeachers = [db, state, finish, runStaff, wantTeachers,
                        qArg]() mutable {
        if (!wantTeachers)
        {
            runStaff();
            return;
        }
        // su.id IS NULL — без staff-карточек (их отдаёт шаг 2).
        const std::string sql =
            kTeacherSelect +
            "WHERE (su.id IS NULL) "
            "AND ($1 = '' OR tp.fio LIKE $1 ESCAPE '\\' "
            "OR tp.title LIKE $1 ESCAPE '\\' "
            "OR tp.department LIKE $1 ESCAPE '\\') ORDER BY tp.id";
        *db << sql << qArg >>
            [state, finish, runStaff](const drogon::orm::Result& rows) mutable {
                for (const auto& r : rows)
                    state->people.append(teacherJson(r));
                runStaff();
            } >>
            [state, finish, runStaff](const DrogonDbException& e) mutable {
                LOG_ERROR << "PeopleService::search(teachers): "
                          << e.base().what();
                state->failed = true;
                runStaff();
            };
    };

    runTeachers();
}

void PeopleService::profile(DbClientPtr db,
                            int64_t requesterUserId,
                            int64_t personId,
                            ResultCb cb)
{
    if (personId >= kPersonIdOffset)
    {
        // Преподаватель/сотрудник — полный профиль (02 §5.2, гайд §9а).
        const int64_t tpId = personId - kPersonIdOffset;
        *db << kTeacherSelect + "WHERE tp.id = $1" << tpId >>
            [cb, personId](const drogon::orm::Result& rows) mutable {
                if (rows.empty())
                {
                    cb(Result{Status::NotFound, Json::Value(), ""});
                    return;
                }
                cb(Result{Status::Ok, teacherJson(rows[0]), ""});
            } >>
            [cb](const DrogonDbException& e) mutable {
                LOG_ERROR << "PeopleService::profile(teacher): "
                          << e.base().what();
                cb(Result{Status::Error, Json::Value(), ""});
            };
        return;
    }

    // Студент: users.role='student' (staff-юзер id<1000 → NotFound — его
    // карточка в каталоге под teacher_profiles id, Р-14/Р-15).
    *db << kStudentFull << personId >>
        [cb, requesterUserId](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                cb(Result{Status::NotFound, Json::Value(), ""});
                return;
            }
            const Row& r = rows[0];
            const bool self = r["id"].as<int64_t>() == requesterUserId;
            Json::Value p(Json::objectValue);
            p["id"] = r["id"].as<int64_t>();
            p["role"] = "student";
            p["fio"] = r["fio"].as<std::string>();
            p["group"] = r["group_name"].as<std::string>();
            p["email"] = studentEmail(r["login"].as<std::string>());
            if (self)
            {
                // Свой — полный (поля /v1/me: UserService::loadProfile).
                p["faculty"] = r["faculty"].as<std::string>();
                p["program"] = r["program"].as<std::string>();
                p["course"] = r["course"].as<int64_t>();
                p["group_id"] = r["group_id"].as<int64_t>();
                p["student_card_no"] = r["student_card_no"].as<std::string>();
            }
            // Чужой — ТОЛЬКО публичные поля (имя, группа, почта): faculty/
            // program/course/group_id/student_card_no и любые прочие
            // персональные данные наружу не уходят (02 §5.2, правила
            // видимости 5.11–5.16). Оценки/заявки/брони этими эндпоинтами
            // не отдаются вовсе — DoD №2 на своих эндпоинтах (шапка).
            cb(Result{Status::Ok, std::move(p), ""});
        } >>
        [cb](const DrogonDbException& e) mutable {
            LOG_ERROR << "PeopleService::profile(student): "
                      << e.base().what();
            cb(Result{Status::Error, Json::Value(), ""});
        };
}

void PeopleService::schedule(DbClientPtr db, int64_t personId, ResultCb cb)
{
    if (personId >= kPersonIdOffset)
    {
        const int64_t tpId = personId - kPersonIdOffset;
        *db << kTeacherSelect + "WHERE tp.id = $1" << tpId >>
            [db, cb, personId, tpId](const drogon::orm::Result& rows) mutable {
                if (rows.empty())
                {
                    cb(Result{Status::NotFound, Json::Value(), ""});
                    return;
                }
                Json::Value person(Json::objectValue);
                person["id"] = personId;
                person["role"] =
                    rows[0]["is_staff"].as<int>() != 0 ? "staff" : "teacher";
                Json::Value body;
                body["person"] = std::move(person);
                if (rows[0]["is_staff"].as<int>() != 0)
                {
                    // У сотрудника занятий нет — честный пустой список.
                    body["events"] = Json::Value(Json::arrayValue);
                    cb(Result{Status::Ok, std::move(body), ""});
                    return;
                }
                // Р-13: teacher_profiles.id выровнен 1:1 с teachers.id
                // фикстурами — schedule_events.teacher_id (FK на
                // teachers(0002)) адресуется напрямую профилем.
                ScheduleService::eventsByTeacher(
                    db, tpId,
                    [cb, body = std::move(body)](
                        bool ok, Json::Value events) mutable {
                        if (!ok)
                        {
                            cb(Result{Status::Error, Json::Value(), ""});
                            return;
                        }
                        body["events"] = std::move(events);
                        cb(Result{Status::Ok, std::move(body), ""});
                    });
            } >>
            [cb](const DrogonDbException& e) mutable {
                LOG_ERROR << "PeopleService::schedule(teacher): "
                          << e.base().what();
                cb(Result{Status::Error, Json::Value(), ""});
            };
        return;
    }

    // Студент: расписание ЕГО группы — публично (01 §5.11 «расписание
    // группы — публичная информация вуза»); requester не проверяется.
    *db << kStudentTarget << personId >>
        [db, cb, personId](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                cb(Result{Status::NotFound, Json::Value(), ""});
                return;
            }
            const int64_t groupId = rows[0]["group_id"].as<int64_t>();
            Json::Value person(Json::objectValue);
            person["id"] = personId;
            person["role"] = "student";
            Json::Value body;
            body["person"] = std::move(person);
            ScheduleService::eventsOnRange(
                db, groupId, kAllFrom, kAllTo,
                [cb, body = std::move(body)](
                    bool ok, Json::Value events) mutable {
                    if (!ok)
                    {
                        cb(Result{Status::Error, Json::Value(), ""});
                        return;
                    }
                    body["events"] = std::move(events);
                    cb(Result{Status::Ok, std::move(body), ""});
                });
        } >>
        [cb](const DrogonDbException& e) mutable {
            LOG_ERROR << "PeopleService::schedule(student): "
                      << e.base().what();
            cb(Result{Status::Error, Json::Value(), ""});
        };
}

}  // namespace para
