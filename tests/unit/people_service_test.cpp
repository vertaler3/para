// Юнит-тесты PeopleService (Э6а шаг 2; 02 §5.2 строки /v1/people* и
// «Правила видимости новых модулей (5.11–5.16)»):
//  - loadRequester — роль/факультет запрашивающего из БД (staff — без
//    профиля);
//  - search — преподаватели и сотрудники видны всем; студенты — только
//    своего факультета (факультет запрашивающего-студента из БД, staff —
//    все + ?faculty=); role вне словаря → BadRequest; q с LIKE-метасимволами
//    не расширяет поиск (ESCAPE '\');
//  - profile — преподаватель/сотрудник полный; студент — свой полный
//    (поля /v1/me + почта), чужой — ТОЛЬКО публичные поля (имя, группа,
//    почта); в чужом профиле нет faculty/program/course/group_id/
//    student_card_no и нет полей оценок/заявок/броней (DoD №2 остаётся на
//    собственных эндпоинтах); staff-юзер под users.id → NotFound (Р-14/15);
//  - schedule — занятия преподавателя по teacher_id (Р-13: выравнивание
//    teacher_profiles.id с teachers.id фикстурами), события с teacher_id
//    (Э6а, NULL → null); расписание группы студента — публично (01 §5.11);
//    у сотрудника — пусто; несуществующие → NotFound;
//  - maskFullName — формат «И. Фамилия» (02 §5.2: «А. Соколова») и его
//    emitUTF8-сериализация (кириллица сырыми байтами — Э6 правки р1);
//  - fail-closed: нет таблицы → Error, не частичный ответ и не падение.
// Данные — срез sql/fixtures.sql (5.11): 4 студента/2 staff, группы
// 1-ФМО (Аня, Олег) / 4-МЭО (Марк) / 4-ФП (Дина); teacher_profiles id 1
// (Кузнецова — преподаватель) и 7 (Валитова — staff-карточка dean.demo);
// teachers (0002) с выравниванием Р-13; события 1–2 — группа 1 (как в
// фикстурах, teacher_id 4/5), событие 3 — группа 2 без преподавателя,
// событие 4 — группа 1 у Кузнецовой (teacher_id 1).
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>
#include <drogon/drogon.h>

#include "services/PeopleService.h"
#include "utils/NameMask.h"

namespace fs = std::filesystem;
using para::PeopleService;
using PStatus = PeopleService::Status;

namespace {

struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-people-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void execFile(sqlite3* db, const std::string& sql)
{
    char* err = nullptr;
    REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
    if (err != nullptr)
    {
        FAIL(err);
        sqlite3_free(err);
    }
}

// Схема — колонки users/groups/profiles (0001), disciplines/schedule_events/
// teachers (0002), teacher_profiles (0006). CHECK-и включены там, где на них
// опирается контракт. FK не включены (как в рантайме — находка ревью Э1).
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withTeacherProfiles = true)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        execFile(raw,
                 "CREATE TABLE users ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  login TEXT NOT NULL UNIQUE,"
                 "  role TEXT NOT NULL CHECK (role IN ('student','staff')));"
                 "CREATE TABLE groups ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL UNIQUE,"
                 "  course INTEGER NOT NULL);"
                 "CREATE TABLE profiles ("
                 "  user_id INTEGER PRIMARY KEY REFERENCES users(id),"
                 "  fio TEXT NOT NULL,"
                 "  faculty TEXT NOT NULL,"
                 "  program TEXT NOT NULL,"
                 "  course INTEGER NOT NULL,"
                 "  group_id INTEGER NOT NULL REFERENCES groups(id),"
                 "  student_card_no TEXT NOT NULL UNIQUE);"
                 "CREATE TABLE disciplines ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  code TEXT NOT NULL UNIQUE,"
                 "  title TEXT NOT NULL,"
                 "  control_type TEXT NOT NULL);"
                 "CREATE TABLE teachers ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  fio TEXT NOT NULL,"
                 "  department TEXT);"
                 "CREATE TABLE schedule_events ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  group_id INTEGER NOT NULL REFERENCES groups(id),"
                 "  discipline_id INTEGER NOT NULL REFERENCES"
                 " disciplines(id),"
                 "  teacher_id INTEGER REFERENCES teachers(id),"
                 "  room TEXT NOT NULL,"
                 "  kind TEXT NOT NULL,"
                 "  starts_at TEXT NOT NULL,"
                 "  ends_at TEXT NOT NULL,"
                 "  status TEXT NOT NULL DEFAULT 'normal',"
                 "  prev_room TEXT,"
                 "  prev_starts_at TEXT,"
                 "  prev_teacher_id INTEGER,"
                 "  changed_at TEXT);"
                 "INSERT INTO users(id, login, role) VALUES"
                 " (1,'anya.demo','student'), (2,'oleg.demo','student'),"
                 " (3,'mark.demo','student'), (4,'dina.demo','student'),"
                 " (5,'dean.demo','staff'), (6,'office.demo','staff');"
                 "INSERT INTO groups(id, name, course) VALUES"
                 " (1,'1-ФМО',1), (2,'4-МЭО',4), (3,'4-ФП',4);"
                 "INSERT INTO profiles(user_id, fio, faculty, program,"
                 " course, group_id, student_card_no) VALUES"
                 " (1,'Соколова Анна Игоревна','Факультет международных"
                 " отношений (ФМО)','Международные отношения',1,1,"
                 " 'ФМО-101-26/001'),"
                 " (2,'Родионов Олег Павлович','Факультет международных"
                 " отношений (ФМО)','Международные отношения',1,1,"
                 " 'ФМО-101-26/042'),"
                 " (3,'Ливанов Марк Сергеевич','Факультет мировой экономики"
                 " и политики','Мировая экономика',4,2,'МЭО-401-23/017'),"
                 " (4,'Хакимова Дина Ринатовна','Факультет политологии"
                 " (ФП)','Сравнительная политология',4,3,'ФП-401-23/005');"
                 "INSERT INTO disciplines(id, code, title, control_type)"
                 " VALUES (1,'ME-101','Мировая экономика','exam'),"
                 " (4,'HR-101','История России','graded_credit');"
                 // Корпуса/аудитории — срез фикстур v2 (0005+0006) для
                 // аудиторий этого теста: 1201 — Корпус Б, 3024 — Корпус А.
                 // Э7-БЕ шаг 3 (8.iv): kSelect ScheduleService LEFT JOIN'ит
                 // rooms→buildings — таблицы обязаны существовать в схеме
                 // теста (иначе Ok-ветки расписания падают «no such table»).
                 "CREATE TABLE buildings (id INTEGER PRIMARY KEY, name TEXT"
                 " NOT NULL, address TEXT NOT NULL);"
                 "INSERT INTO buildings VALUES"
                 " (1,'Корпус А','проспект Научный, д. 10 (адрес"
                 " вымышленный)'),"
                 " (2,'Корпус Б','улица Парковая, д. 4 (адрес"
                 " вымышленный)');"
                 "CREATE TABLE rooms (id INTEGER PRIMARY KEY, name TEXT NOT"
                 " NULL, building TEXT NOT NULL, capacity INTEGER NOT NULL,"
                 " features TEXT NOT NULL DEFAULT '', building_id INTEGER,"
                 " floor INTEGER, kind TEXT);"
                 "INSERT INTO rooms(id, name, building, capacity, features,"
                 " building_id, floor, kind) VALUES"
                 " (9,'3024','Корпус А',32,'проектор, доска',1,3,"
                 "'аудитория'),"
                 " (22,'1201','Корпус Б',32,'проектор, доска',2,2,"
                 "'аудитория');"
                 "INSERT INTO schedule_events(id, group_id, discipline_id,"
                 " teacher_id, room, kind, starts_at, ends_at, status) VALUES"
                 " (1,1,4,4,'1201','lecture','2026-11-02T10:00:00Z',"
                 "  '2026-11-02T11:30:00Z','normal'),"
                 " (2,1,4,4,'1201','lecture','2026-11-02T14:00:00Z',"
                 "  '2026-11-02T15:30:00Z','normal'),"
                 " (3,2,1,NULL,'3024','seminar','2026-11-03T10:00:00Z',"
                 "  '2026-11-03T11:30:00Z','normal');");
        if (withTeacherProfiles)
        {
            execFile(raw,
                     "CREATE TABLE teacher_profiles ("
                     "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "  fio TEXT NOT NULL,"
                     "  title TEXT,"
                     "  department TEXT,"
                     "  email TEXT NOT NULL UNIQUE,"
                     "  office TEXT);"
                     "INSERT INTO teacher_profiles(id, fio, title,"
                     " department, email, office) VALUES"
                     " (1,'Кузнецова Марина Павловна','Доцент',"
                     "  'Кафедра мировой экономики','m.kuznetsova@example"
                     ".edu','Корпус А, ауд. 3032'),"
                     " (7,'Валитова Оксана Борисовна','Сотрудник деканата',"
                     "  'Деканат ФМО (вымышленный)','dean.demo@example"
                     ".edu','Корпус А, ауд. 3110');");
        }
        // teachers (0002) — FK-якорь teacher_id; выравнивание Р-13
        // (teachers.id 1 == teacher_profiles.id 1, fio согласованы).
        execFile(raw,
                 "INSERT INTO teachers(id, fio, department) VALUES"
                 " (1,'Кузнецова Марина Павловна','Кафедра мировой"
                 " экономики'),"
                 " (7,'Валитова Оксана Борисовна','Деканат ФМО');");
        // Событие Кузнецовой (teacher_id 1) — группа 1, отдельно от базовых.
        execFile(raw,
                 "INSERT INTO schedule_events(id, group_id, discipline_id,"
                 " teacher_id, room, kind, starts_at, ends_at, status) VALUES"
                 " (4,1,1,1,'3024','seminar','2026-11-03T10:00:00Z',"
                 "  '2026-11-03T11:30:00Z','normal');");
        sqlite3_close(raw);
        // FK на каждом соединении (Э8 шаг 5, находка Э1) — те же
        // initialization_queries, что в проде (config.json); Drogon
        // 1.9.14 исполняет их на каждом новом соединении пула.
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }
};

// Синхронный вызов сервисного метода: колбэк выполняется в потоке БД,
// результат — через promise (паттерн rooms_service_test).
PeopleService::Requester requesterOf(Fixture& f, int64_t userId)
{
    std::promise<std::pair<PStatus, PeopleService::Requester>> p;
    auto fut = p.get_future();
    PeopleService::loadRequester(
        f.db, userId, [&p](PStatus s, PeopleService::Requester r) {
            p.set_value({s, std::move(r)});
        });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    auto [s, r] = fut.get();
    REQUIRE(s == PStatus::Ok);
    return r;
}

PeopleService::Result searchOf(Fixture& f,
                               int64_t requesterId,
                               PeopleService::SearchQuery q)
{
    auto req = requesterOf(f, requesterId);
    std::promise<PeopleService::Result> p;
    auto fut = p.get_future();
    PeopleService::search(
        f.db, std::move(req), std::move(q),
        [&p](PeopleService::Result r) { p.set_value(std::move(r)); });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return fut.get();
}

PeopleService::Result profileOf(Fixture& f,
                                int64_t requesterId,
                                int64_t personId)
{
    std::promise<PeopleService::Result> p;
    auto fut = p.get_future();
    PeopleService::profile(
        f.db, requesterId, personId,
        [&p](PeopleService::Result r) { p.set_value(std::move(r)); });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return fut.get();
}

PeopleService::Result scheduleOf(Fixture& f, int64_t personId)
{
    std::promise<PeopleService::Result> p;
    auto fut = p.get_future();
    PeopleService::schedule(
        f.db, personId,
        [&p](PeopleService::Result r) { p.set_value(std::move(r)); });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return fut.get();
}

}  // namespace

TEST_CASE("loadRequester: роль и факультет из БД; staff без профиля")
{
    Fixture f;
    const auto anya = requesterOf(f, 1);
    CHECK(anya.role == "student");
    REQUIRE(anya.faculty.has_value());
    CHECK(*anya.faculty == "Факультет международных отношений (ФМО)");
    const auto dean = requesterOf(f, 5);
    CHECK(dean.role == "staff");
    CHECK_FALSE(dean.faculty.has_value());
}

TEST_CASE("search: преподаватели и сотрудники — всем; студенты — свой "
          "факультет (02 §5.2)")
{
    Fixture f;
    // Аня (студент ФМО) без role: Кузнецова (teacher), Валитова (staff),
    // студенты ФМО — Аня и Олег; Марк (МЭО) и Дина (ФП) НЕ попадают.
    auto r = searchOf(f, 1, {});
    REQUIRE(r.status == PStatus::Ok);
    const auto& people = r.body["people"];
    REQUIRE(people.size() == 4);
    CHECK(people[0]["role"].asString() == "teacher");
    CHECK(people[0]["id"].as<int64_t>() == 1001);  // Р-15: 1 + 1000
    CHECK(people[0]["email"].asString() == "m.kuznetsova@example.edu");
    CHECK(people[1]["role"].asString() == "staff");
    CHECK(people[1]["id"].as<int64_t>() == 1007);
    CHECK(people[1]["email"].asString() == "dean.demo@example.edu");
    CHECK(people[2]["role"].asString() == "student");
    CHECK(people[2]["id"].as<int64_t>() == 1);
    CHECK(people[2]["group"].asString() == "1-ФМО");
    CHECK(people[2]["email"].asString() == "anya.demo@example.edu");
    CHECK(people[3]["id"].as<int64_t>() == 2);  // Олег, та же группа
    // Гейт Э6а р1: полное ФИО чужих наружу не уходит — в search чужие
    // студенты маскированы (Аня видит себя полной, Олега — «О. Родионов»).
    CHECK(people[2]["fio"].asString() == "Соколова Анна Игоревна");
    CHECK(people[3]["fio"].asString() == "О. Родионов");
    for (const auto& p : people)
        CHECK(p["fio"].asString() != "Ливанов Марк Сергеевич");
}

TEST_CASE("search: staff видит всех студентов; role=student/teacher/staff; "
          "?faculty= — фильтр по студентам")
{
    Fixture f;
    // dean (staff) — студенты всех факультетов: 4.
    auto r = searchOf(f, 5, {});
    REQUIRE(r.status == PStatus::Ok);
    int students = 0;
    for (const auto& p : r.body["people"])
        if (p["role"].asString() == "student")
            ++students;
    CHECK(students == 4);

    PeopleService::SearchQuery teacher;
    teacher.role = "teacher";
    auto rt = searchOf(f, 1, teacher);
    REQUIRE(rt.status == PStatus::Ok);
    REQUIRE(rt.body["people"].size() == 1);  // staff-карточка не подмешана
    CHECK(rt.body["people"][0]["fio"].asString() ==
          "Кузнецова Марина Павловна");
    CHECK(rt.body["people"][0]["office"].asString() ==
          "Корпус А, ауд. 3032");

    PeopleService::SearchQuery staff;
    staff.role = "staff";
    auto rs = searchOf(f, 1, staff);
    REQUIRE(rs.status == PStatus::Ok);
    REQUIRE(rs.body["people"].size() == 1);
    CHECK(rs.body["people"][0]["fio"].asString() ==
          "Валитова Оксана Борисовна");

    PeopleService::SearchQuery facultyOnly;
    facultyOnly.role = "student";
    facultyOnly.faculty = "Факультет мировой экономики и политики";
    auto rf = searchOf(f, 5, facultyOnly);  // staff запросил — фильтр применим
    REQUIRE(rf.status == PStatus::Ok);
    REQUIRE(rf.body["people"].size() == 1);
    CHECK(rf.body["people"][0]["id"].as<int64_t>() == 3);  // Марк
    // У staff нет «своей» строки — чужой студент маскирован (гейт Э6а р1).
    CHECK(rf.body["people"][0]["fio"].asString() == "М. Ливанов");

    // Студент НЕ может расширить факультет через ?faculty= (свой — из БД).
    PeopleService::SearchQuery widen;
    widen.role = "student";
    widen.faculty = "Факультет мировой экономики и политики";
    auto rw = searchOf(f, 1, widen);  // Аня просит чужой факультет
    REQUIRE(rw.status == PStatus::Ok);
    for (const auto& p : rw.body["people"])
        CHECK(p["id"].as<int64_t>() != 3);  // Марк всё ещё не виден
}

TEST_CASE("search: q с LIKE-метасимволами не расширяет поиск; "
          "role вне словаря → BadRequest")
{
    Fixture f;
    PeopleService::SearchQuery dots;
    dots.role = "teacher";
    dots.q = "%";
    auto r = searchOf(f, 1, dots);
    REQUIRE(r.status == PStatus::Ok);
    CHECK(r.body["people"].size() == 0);  // '%' экранирован — буквально

    PeopleService::SearchQuery prefix;
    prefix.role = "teacher";
    prefix.q = "Кузнец";
    auto rp = searchOf(f, 1, prefix);
    REQUIRE(rp.status == PStatus::Ok);
    REQUIRE(rp.body["people"].size() == 1);
    CHECK(rp.body["people"][0]["id"].as<int64_t>() == 1001);

    PeopleService::SearchQuery bogus;
    bogus.role = "bogus";
    auto rb = searchOf(f, 1, bogus);
    CHECK(rb.status == PStatus::BadRequest);
}

TEST_CASE("profile: преподаватель и сотрудник — полный; "
          "несуществующие id → NotFound")
{
    Fixture f;
    auto r = profileOf(f, 1, 1001);
    REQUIRE(r.status == PStatus::Ok);
    const auto& prof = r.body;
    CHECK(prof["id"].as<int64_t>() == 1001);
    CHECK(prof["role"].asString() == "teacher");
    CHECK(prof["fio"].asString() == "Кузнецова Марина Павловна");
    CHECK(prof["title"].asString() == "Доцент");
    CHECK(prof["department"].asString() == "Кафедра мировой экономики");
    CHECK(prof["email"].asString() == "m.kuznetsova@example.edu");
    CHECK(prof["office"].asString() == "Корпус А, ауд. 3032");

    auto rs = profileOf(f, 1, 1007);  // карточка сотрудника — полный
    REQUIRE(rs.status == PStatus::Ok);
    CHECK(rs.body["role"].asString() == "staff");
    CHECK(rs.body["email"].asString() == "dean.demo@example.edu");

    CHECK(profileOf(f, 1, 1002).status == PStatus::NotFound);
    CHECK(profileOf(f, 1, 999).status == PStatus::NotFound);
}

TEST_CASE("profile: студент — свой полный, чужой — только публичные поля")
{
    Fixture f;
    auto self = profileOf(f, 3, 3);  // Марк смотрит себя
    REQUIRE(self.status == PStatus::Ok);
    const auto& own = self.body;
    CHECK(own["role"].asString() == "student");
    CHECK(own["fio"].asString() == "Ливанов Марк Сергеевич");
    CHECK(own["faculty"].asString() ==
          "Факультет мировой экономики и политики");
    CHECK(own["program"].asString() == "Мировая экономика");
    CHECK(own["course"].as<int64_t>() == 4);
    CHECK(own["group"].asString() == "4-МЭО");
    CHECK(own["group_id"].as<int64_t>() == 2);
    CHECK(own["student_card_no"].asString() == "МЭО-401-23/017");
    CHECK(own["email"].asString() == "mark.demo@example.edu");  // Р-15

    auto other = profileOf(f, 1, 3);  // Аня смотрит Марка
    REQUIRE(other.status == PStatus::Ok);
    const auto& pub = other.body;
    CHECK(pub["id"].as<int64_t>() == 3);
    CHECK(pub["role"].asString() == "student");
    // Гейт Э6а р1: чужому — МАСКИРОВАННОЕ имя, полного ФИО в ответе нет.
    CHECK(pub["fio"].asString() == "М. Ливанов");
    CHECK(pub["group"].asString() == "4-МЭО");
    CHECK(pub["email"].asString() == "mark.demo@example.edu");
    CHECK_FALSE(pub.isMember("faculty"));
    CHECK_FALSE(pub.isMember("program"));
    CHECK_FALSE(pub.isMember("course"));
    CHECK_FALSE(pub.isMember("group_id"));
    CHECK_FALSE(pub.isMember("student_card_no"));
    // Полное имя чужого наружу не уходит вовсе (02 §5.2 строка 603 —
    // правило всего каталога, гейт-чек «безОценок»).
    Json::StreamWriterBuilder bMask;
    bMask["emitUTF8"] = true;
    bMask["indentation"] = "";
    const std::string pubRaw = Json::writeString(bMask, pub);
    CHECK(pubRaw.find("Ливанов Марк Сергеевич") == std::string::npos);
    // Оценки/заявки/брони чужих — никогда: в схеме ответа таких полей нет.
    CHECK_FALSE(pub.isMember("grades"));
    CHECK_FALSE(pub.isMember("applications"));
    CHECK_FALSE(pub.isMember("room_bookings"));
}

TEST_CASE("profile: staff-юзер под users.id → NotFound (карточка Р-14)")
{
    Fixture f;
    // dean.demo (users id 5) — не студент; карточка в каталоге — 1007.
    CHECK(profileOf(f, 1, 5).status == PStatus::NotFound);
    CHECK(profileOf(f, 1, 6).status == PStatus::NotFound);
}

TEST_CASE("schedule: занятия преподавателя; группа студента публично; "
          "сотрудник — пусто; NotFound")
{
    Fixture f;
    // Преподаватель 1001 (teacher_profiles 1 == teachers 1, Р-13): событие 4.
    auto rt = scheduleOf(f, 1001);
    REQUIRE(rt.status == PStatus::Ok);
    CHECK(rt.body["person"]["role"].asString() == "teacher");
    REQUIRE(rt.body["events"].size() == 1);
    CHECK(rt.body["events"][0]["id"].as<int64_t>() == 4);
    CHECK(rt.body["events"][0]["teacher_id"].as<int64_t>() ==
          1);  // Э6а: schedule-ответы дополняются teacher_id
    // Э7-БЕ шаг 3 (8.iv): форма события schedule-выборок дополнилась
    // корпусом — ауд. 3024 → «Корпус А» (LEFT JOIN rooms→buildings).
    CHECK(rt.body["events"][0]["building"].asString() == "Корпус А");
    CHECK(rt.body["events"][0]["prev_building"].isNull());

    // Чужой студент: расписание группы 4-МЭО Марка (событие 3) — публично.
    auto rm = scheduleOf(f, 3);
    REQUIRE(rm.status == PStatus::Ok);
    CHECK(rm.body["person"]["role"].asString() == "student");
    REQUIRE(rm.body["events"].size() == 1);
    CHECK(rm.body["events"][0]["id"].as<int64_t>() == 3);
    CHECK(rm.body["events"][0]["teacher_id"].isNull());  // NULL → null

    // Группа Ани/Олега (события 1, 2, 4 — по времени).
    auto ra = scheduleOf(f, 1);
    REQUIRE(ra.status == PStatus::Ok);
    REQUIRE(ra.body["events"].size() == 3);
    CHECK(ra.body["events"][0]["id"].as<int64_t>() == 1);
    CHECK(ra.body["events"][2]["id"].as<int64_t>() == 4);

    // Сотрудник (1007) — занятий нет, честный пустой список.
    auto rst = scheduleOf(f, 1007);
    REQUIRE(rst.status == PStatus::Ok);
    CHECK(rst.body["person"]["role"].asString() == "staff");
    CHECK(rst.body["events"].size() == 0);

    // Несуществующие/не-студент → NotFound.
    CHECK(scheduleOf(f, 1002).status == PStatus::NotFound);
    CHECK(scheduleOf(f, 999).status == PStatus::NotFound);
    CHECK(scheduleOf(f, 5).status == PStatus::NotFound);
}

TEST_CASE("maskFullName: формат «И. Фамилия» 02 §5.2 и emitUTF8-сериализация")
{
    using para::maskFullName;
    CHECK(maskFullName("Соколова Анна Игоревна") == "А. Соколова");
    CHECK(maskFullName("Ливанов Марк Сергеевич") == "М. Ливанов");
    CHECK(maskFullName("Кузнецова Марина Павловна") == "М. Кузнецова");
    CHECK(maskFullName("Хакимова Дина Ринатовна") == "Д. Хакимова");
    CHECK(maskFullName("Родионов Олег Павлович") == "О. Родионов");
    CHECK(maskFullName("") == "");
    CHECK(maskFullName("Однослово") == "Однослово");

    // Сериализация в JSON-ответ (emitUTF8, как api::jsonResponse): кириллица
    // остаётся сырыми UTF-8 байтами — «escaped»-подстрока для гейт-поиска.
    Json::StreamWriterBuilder b;
    b["emitUTF8"] = true;
    b["indentation"] = "";
    const std::string body =
        Json::writeString(b, maskFullName("Ливанов Марк Сергеевич"));
    CHECK(body.find("М. Ливанов") != std::string::npos);
    CHECK(body.find("\\u") == std::string::npos);  // без \u-эскейпов
}

// Рейтинговая маска (запрос владельца 22.11): чужие в рейтинге — только
// инициал имени и фиксированные «****»: фамилии и её длины нет вовсе
// («М. Титова» → «М****»). Каталог людей маскируется по-прежнему
// maskFullName'ом — там поиск требует узнаваемости.
TEST_CASE("maskRatingName: инициал + «****» — без фамилии и её длины")
{
    using para::maskRatingName;
    CHECK(maskRatingName("Титова Маргарита Денисовна") == "М****");
    CHECK(maskRatingName("Ливанов Марк Сергеевич") == "М****");
    CHECK(maskRatingName("Соколова Анна Игоревна") == "А****");
    CHECK(maskRatingName("Хакимова Дина Ринатовна") == "Д****");
    // Разная длина фамилий — маска одинаковой длины (длину не выдаём).
    CHECK(maskRatingName("Иванов Пётр").size() ==
          maskRatingName("Богдановский Александр Николаевич").size());
    CHECK(maskRatingName("Однослово") == "О****");  // инициал первого слова
    CHECK(maskRatingName("") == "");
    CHECK(maskRatingName("   ") == "");
}

TEST_CASE("fail-closed: нет таблицы teacher_profiles → Error, не падение")
{
    Fixture f(/*withTeacherProfiles=*/false);
    PeopleService::SearchQuery q;
    q.role = "teacher";
    CHECK(searchOf(f, 1, q).status == PStatus::Error);
    CHECK(profileOf(f, 1, 1001).status == PStatus::Error);
    CHECK(scheduleOf(f, 1001).status == PStatus::Error);
    // Студенческие ветки (users/profiles) живы — работают.
    CHECK(scheduleOf(f, 3).status == PStatus::Ok);
}
