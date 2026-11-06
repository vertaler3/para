#include "services/ScheduleChangeService.h"

#include <atomic>
#include <cstdio>
#include <ctime>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <drogon/drogon.h>

#include "services/NotifyDispatcher.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Событие + дисциплина + ФИО преподавателя (для тел уведомлений).
const std::string kEventSql =
    "SELECT e.id, e.group_id, e.discipline_id, e.teacher_id, e.room, "
    "       e.kind, e.starts_at, e.ends_at, e.status, e.prev_starts_at, "
    "       e.prev_room, e.prev_teacher_id, e.changed_at, "
    "       d.title AS discipline_title, d.code AS discipline_code "
    "FROM schedule_events e "
    "JOIN disciplines d ON d.id = e.discipline_id "
    "WHERE e.id = $1";

// Условный UPDATE: совпадение id и ТЕКУЩЕГО статуса в WHERE — защита от
// гонки параллельного изменения (0 строк → BadRequest, ничего не изменено).
// prev_* — «было → стало» (миграция 0002). Повторное изменение перезаписывает
// prev_* текущими значениями — предыдущей истории прототип не хранит
// (зафиксировано в журнале шага).
const std::string kUpdateSql =
    "UPDATE schedule_events SET status = $1, starts_at = $2, ends_at = $3, "
    "room = $4, teacher_id = $5, prev_starts_at = $6, prev_room = $7, "
    "prev_teacher_id = $8, changed_by = $9, changed_at = $10 "
    "WHERE id = $11 AND status = $12";

// Студенты группы события — получатели авторассылки (profiles — 0001).
const std::string kRecipientsSql =
    "SELECT user_id FROM profiles WHERE group_id = $1";

// Существование замещающего преподавателя (teachers — 0002).
const std::string kTeacherSql = "SELECT id FROM teachers WHERE id = $1";

// cb копируется в ОБЕ ветки каждой цепочки (урок Э2 шаг 1: move в
// success-ветку оставил бы error-ветке пустой std::function → bad_function_call).

// ISO «YYYY-MM-DDTHH:MM:SSZ» → time_t (UTC, timegm); некалендарная → -1
// (обратная сборка строки ловит нормализацию timegm — приём validDate).
std::time_t isoToTime(const std::string& iso)
{
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &s) != 6)
        return -1;
    if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 ||
        mi > 59 || s > 59)
        return -1;
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = s;
    tm.tm_isdst = 0;
    const std::time_t t = timegm(&tm);
    std::tm back{};
    gmtime_r(&t, &back);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &back);
    return (iso == buf) ? t : -1;
}

std::string timeToIso(std::time_t t)
{
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

// YYYY-MM-DD из ISO-метки — deep_link → карточка переноса конкретного дня.
std::string datePartOf(const std::string& iso)
{
    return iso.size() >= 10 ? iso.substr(0, 10) : iso;
}

// Состояние события, читаемое до/после изменения.
struct EventRow
{
    int64_t id = 0;
    int64_t groupId = 0;
    std::string startsAt;
    std::string endsAt;
    std::string room;
    std::optional<int64_t> teacherId;
    std::string status;
    std::string discipline;

    static EventRow of(const Row& r)
    {
        EventRow ev;
        ev.id = r["id"].as<int64_t>();
        ev.groupId = r["group_id"].as<int64_t>();
        ev.startsAt = r["starts_at"].as<std::string>();
        ev.endsAt = r["ends_at"].as<std::string>();
        ev.room = r["room"].as<std::string>();
        if (!r["teacher_id"].isNull())
            ev.teacherId = r["teacher_id"].as<int64_t>();
        ev.status = r["status"].as<std::string>();
        ev.discipline = r["discipline_title"].as<std::string>();
        return ev;
    }
};

// JSON события в формате 02 §5.2 (алиасы колонок как в ScheduleService).
Json::Value eventJson(const Row& r)
{
    Json::Value e(Json::objectValue);
    e["id"] = r["id"].as<int64_t>();
    e["discipline"] = r["discipline_title"].as<std::string>();
    e["code"] = r["discipline_code"].as<std::string>();
    e["kind"] = r["kind"].as<std::string>();
    e["room"] = r["room"].as<std::string>();
    e["starts_at"] = r["starts_at"].as<std::string>();
    e["ends_at"] = r["ends_at"].as<std::string>();
    e["status"] = r["status"].as<std::string>();
    e["prev_room"] = r["prev_room"].isNull()
                         ? Json::Value()
                         : Json::Value(r["prev_room"].as<std::string>());
    e["prev_starts_at"] =
        r["prev_starts_at"].isNull()
            ? Json::Value()
            : Json::Value(r["prev_starts_at"].as<std::string>());
    e["prev_teacher_id"] =
        r["prev_teacher_id"].isNull()
            ? Json::Value()
            : Json::Value(r["prev_teacher_id"].as<int64_t>());
    e["changed_at"] = r["changed_at"].isNull()
                          ? Json::Value()
                          : Json::Value(r["changed_at"].as<std::string>());
    return e;
}

// Подготовка уведомления ДО транзакции: заголовок/тело/deep_link/срочность
// известны из тела запроса и старого состояния события.
NotifyInput buildNotice(const ScheduleChangeService::Input& in,
                        const EventRow& ev,
                        const std::string& newStarts,
                        const std::string& newRoom)
{
    NotifyInput n;
    n.type = NotifyType::Schedule;
    std::string body;
    switch (in.action)
    {
        case ScheduleChangeService::Action::Move:
            n.title = "Пара перенесена: " + ev.discipline;
            body = "Было: " + ev.startsAt + " ауд. " + ev.room + " → станет: " +
                   newStarts + " ауд. " + newRoom;
            break;
        case ScheduleChangeService::Action::Cancel:
            n.title = "Пара отменена: " + ev.discipline;
            body = "Занятие " + ev.startsAt + " (ауд. " + ev.room +
                   ") отменено";
            break;
        case ScheduleChangeService::Action::Substitute:
            n.title = "Замена преподавателя: " + ev.discipline;
            body = "Преподаватель занятия " + ev.startsAt + " заменён";
            break;
    }
    if (!in.reason.empty())
        body += ". Причина: " + in.reason;
    n.body = body;
    // Тап → карточка переноса (день ЦЕЛЕВОГО начала: move — новый, иначе
    // прежний). deep_link заполняется всегда (02 §4.1).
    const std::string& target =
        in.action == ScheduleChangeService::Action::Move ? newStarts
                                                         : ev.startsAt;
    n.deepLink = "/schedule?date=" + datePartOf(target);
    // Срочность «ближайшей пары» (01 §5.6): изменение события, начинающегося
    // в течение 24 ч, тихие часы не задерживают.
    n.eventStartsAtUtc = target;
    return n;
}

}  // namespace

std::optional<ScheduleChangeService::Action>
ScheduleChangeService::actionFromString(const std::string& s)
{
    if (s == "move")
        return Action::Move;
    if (s == "cancel")
        return Action::Cancel;
    if (s == "substitute")
        return Action::Substitute;
    return std::nullopt;  // вне словаря 02 §5.2
}

const char* ScheduleChangeService::actionName(Action action)
{
    switch (action)
    {
        case Action::Move:
            return "move";
        case Action::Cancel:
            return "cancel";
        case Action::Substitute:
            return "substitute";
    }
    return "move";
}

bool ScheduleChangeService::validIsoUtc(const std::string& iso)
{
    return isoToTime(iso) >= 0;
}

void ScheduleChangeService::applyChange(DbClientPtr db,
                                        const SessionInfo& staff,
                                        const Input& in,
                                        InvalidateFn invalidate,
                                        ResultCb cb)
{
    if (!db)
    {
        cb(Result{Outcome::Error, {}});
        return;
    }

    // 1. Событие (404 — нет) и его группа.
    *db << kEventSql << in.eventId >>
        [db, staff, in, invalidate, cb = std::move(cb)](
            const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                cb(Result{Outcome::NotFound, {}});
                return;
            }
            const EventRow ev = EventRow::of(rows[0]);

            // 2. Валидация действия по телу и текущему состоянию.
            std::string newStarts = ev.startsAt;
            std::string newRoom = ev.room;
            std::string newEnds = ev.endsAt;
            std::optional<int64_t> newTeacher;
            std::string newStatus;
            const std::string nowIso = NotifyDispatcher::nowIsoUtc();

            switch (in.action)
            {
                case Action::Move:
                {
                    // Нужно хотя бы одно новое значение (02 §5.2).
                    if (!in.startsAt.has_value() && !in.room.has_value())
                    {
                        cb(Result{Outcome::BadRequest, {}});
                        return;
                    }
                    if (in.startsAt.has_value() && !validIsoUtc(*in.startsAt))
                    {
                        cb(Result{Outcome::BadRequest, {}});
                        return;
                    }
                    newStarts = in.startsAt.value_or(ev.startsAt);
                    newRoom = in.room.value_or(ev.room);
                    // ends_at сдвигается на дельту переноса (длительность
                    // пары сохраняется; смена только аудитории — без сдвига).
                    const std::time_t oldT = isoToTime(ev.startsAt);
                    const std::time_t newT = isoToTime(newStarts);
                    const std::time_t oldEnd = isoToTime(ev.endsAt);
                    if (oldT < 0 || newT < 0 || oldEnd < 0)
                    {
                        cb(Result{Outcome::Error, {}});
                        return;
                    }
                    newEnds = timeToIso(oldEnd + (newT - oldT));
                    newStatus = "moved";
                    break;
                }
                case Action::Cancel:
                    // Повторная отмена — недопустимое действие (400).
                    if (ev.status == "cancelled")
                    {
                        cb(Result{Outcome::BadRequest, {}});
                        return;
                    }
                    newStatus = "cancelled";
                    break;
                case Action::Substitute:
                    // Замещающий обязателен и должен существовать (словарь).
                    if (!in.teacherId.has_value() || *in.teacherId <= 0)
                    {
                        cb(Result{Outcome::BadRequest, {}});
                        return;
                    }
                    newTeacher = in.teacherId;
                    newStatus = "substituted";
                    break;
            }

            // 3. Замещающий существует? (только substitute; до транзакции —
            //    тело с несуществующим id — ошибка тела 400.)
            const NotifyInput notice =
                buildNotice(in, ev, newStarts, newRoom);

            auto onTeacherChecked =
                [db, staff, in, ev, invalidate, newStarts, newRoom, newEnds,
                 newTeacher, newStatus, nowIso, notice,
                 cb = std::move(cb)](bool teacherOk) mutable {
                    if (!teacherOk)
                    {
                        cb(Result{Outcome::BadRequest, {}});
                        return;
                    }

                    // 4. Студенты группы — получатели (состав не зависит от
                    //    переноса; читается до транзакции).
                    *db << kRecipientsSql << ev.groupId >>
                        [db, staff, in, ev, invalidate, newStarts, newRoom,
                         newEnds, newTeacher, newStatus, nowIso, notice,
                         cb = std::move(cb)](const drogon::orm::Result& recipients) mutable {
                            std::vector<int64_t> userIds;
                            userIds.reserve(recipients.size());
                            for (const auto& r : recipients)
                                userIds.push_back(r["user_id"].as<int64_t>());

                            // 5. Транзакция: условный UPDATE + перечитывание
                            //    события; после коммита — инвалидация кэша
                            //    недели группы и запуск авторассылки.
                            auto once =
                                std::make_shared<std::atomic_bool>(false);
                            auto fire = [cb, once](Outcome outcome,
                                                   Json::Value body) {
                                bool expected = false;
                                if (!once->compare_exchange_strong(expected,
                                                                   true))
                                    return;
                                cb(Result{outcome, std::move(body)});
                            };

                            db->newTransactionAsync(
                                [db, staff, in, ev, invalidate, newStarts,
                                 newRoom, newEnds, newTeacher, newStatus,
                                 nowIso, notice, userIds = std::move(userIds),
                                 fire](const std::shared_ptr<Transaction>&
                                           tx) mutable {
                                    if (!tx)
                                    {
                                        LOG_ERROR
                                            << "ScheduleChangeService: "
                                               "транзакция недоступна";
                                        fire(Outcome::Error, {});
                                        return;
                                    }
                                    const std::function<void()> fail =
                                        [fire, tx]() {
                                            fire(Outcome::Error, {});
                                            tx->rollback();
                                        };

                                    // prev_* — «было → стало».
                                    std::optional<std::string> prevStarts;
                                    std::optional<std::string> prevRoom;
                                    std::optional<int64_t> prevTeacher;
                                    if (in.action == Action::Move ||
                                        in.action == Action::Cancel)
                                    {
                                        prevStarts = ev.startsAt;
                                        prevRoom = ev.room;
                                    }
                                    else
                                    {
                                        prevTeacher = ev.teacherId;
                                    }

                                    *tx << kUpdateSql
                                        << newStatus << newStarts << newEnds
                                        << newRoom << newTeacher << prevStarts
                                        << prevRoom << prevTeacher
                                        << staff.userId << nowIso << ev.id
                                        << ev.status >>
                                        [tx, db, ev, in, invalidate, notice,
                                         userIds, fire,
                                         fail](
                                            const drogon::orm::Result& r) mutable {
                                            if (r.affectedRows() == 0)
                                            {
                                                LOG_WARN
                                                    << "ScheduleChangeService("
                                                    << ev.id
                                                    << "): изменено параллельно";
                                                fire(Outcome::BadRequest, {});
                                                tx->rollback();
                                                return;
                                            }
                                            // Новое состояние события — в
                                            // транзакции (соединение закреп-
                                            // лено; урок Э5а step 3).
                                            *tx << kEventSql << ev.id >>
                                                [tx, db, in, ev, invalidate,
                                                 notice, userIds, fire](
                                                    const drogon::orm::Result&
                                                        afterRows) mutable {
                                                    if (afterRows.empty())
                                                    {
                                                        fire(Outcome::Error,
                                                             {});
                                                        tx->rollback();
                                                        return;
                                                    }
                                                    Json::Value eventBody =
                                                        eventJson(afterRows[0]);

                                                    // Commit-callback: успех —
                                                    // инвалидация кэша + рас-
                                                    // сылка + 201.
                                                    tx->setCommitCallback(
                                                        [db, ev, in, invalidate,
                                                         notice,
                                                         userIds = std::move(
                                                             userIds),
                                                         eventBody = std::move(
                                                             eventBody),
                                                         fire](bool committed) mutable {
                                                            if (!committed)
                                                            {
                                                                fire(Outcome::Error,
                                                                     {});
                                                                return;
                                                            }
                                                            // Хук Э3:
                                                            // неделя группы
                                                            // больше не
                                                            // актуальна
                                                            // (erase).
                                                            if (invalidate)
                                                                invalidate(
                                                                    ev.groupId);

                                                            // Авторассылка:
                                                            // КАЖДОМУ студенту
                                                            // группы (фоново;
                                                            // тумблеры/тихие
                                                            // часы/срочность
                                                            // решит диспетчер
                                                            // пер-получателя).
                                                            NotifyInput n =
                                                                notice;
                                                            for (const int64_t
                                                                     uid :
                                                                 userIds)
                                                            {
                                                                n.userId = uid;
                                                                NotifyDispatcher::notify(
                                                                    db, n,
                                                                    [](NotifyOutcome) {
                                                                        // Исход
                                                                        // решён
                                                                        // диспет-
                                                                        // чером;
                                                                        // 201 уже
                                                                        // отдан.
                                                                    });
                                                            }

                                                            Json::Value body(
                                                                Json::objectValue);
                                                            body["action"] =
                                                                ScheduleChangeService::
                                                                    actionName(
                                                                        in.action);
                                                            body["event"] =
                                                                eventBody;
                                                            body["recipients"] =
                                                                static_cast<
                                                                    Json::Int64>(
                                                                    userIds
                                                                        .size());
                                                            fire(Outcome::Ok,
                                                                 std::move(
                                                                     body));
                                                        });
                                                } >>
                                                [tx, fail](const DrogonDbException&
                                                               e) {
                                                    LOG_ERROR
                                                        << "ScheduleChangeService: re-select: "
                                                        << e.base().what();
                                                    fail();
                                                };
                                        } >>
                                        [tx, fire,
                                         ev](const DrogonDbException& e) {
                                            LOG_ERROR
                                                << "ScheduleChangeService: update: "
                                                << e.base().what();
                                            fire(Outcome::Error, {});
                                            tx->rollback();
                                        };
                                });
                        } >>
                        [cb](const DrogonDbException& e) {
                            LOG_ERROR << "ScheduleChangeService: recipients: "
                                      << e.base().what();
                            cb(Result{Outcome::Error, {}});
                        };
                };

            if (in.action != Action::Substitute)
            {
                onTeacherChecked(true);
                return;
            }
            // Замещающий должен существовать (400 — ошибка тела).
            *db << kTeacherSql << *in.teacherId >>
                [onTeacherChecked = std::move(onTeacherChecked)](
                    const drogon::orm::Result& trows) mutable {
                    onTeacherChecked(!trows.empty());
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "ScheduleChangeService: teacher: "
                              << e.base().what();
                    cb(Result{Outcome::Error, {}});
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ScheduleChangeService: select event: "
                      << e.base().what();
            cb(Result{Outcome::Error, {}});
        };
}

}  // namespace para
