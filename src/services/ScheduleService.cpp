#include "services/ScheduleService.h"

#include <cstdio>
#include <ctime>
#include <string>

#include <drogon/drogon.h>

#include "services/ScheduleCache.h"

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// SELECT-часть одна для всех выборок: поля события в JSON 02 §5.2 +
// название/код дисциплины (карточка пары). Э6а (гайд BE §9а): schedule-
// ответы дополняются teacher_id — FE вешает ссылку на профиль человека
// (/v1/people/{id}) из «Деталей пары» (5.11); NULL → null (DDL допускает).
// prev_teacher_id — часть «было → стало» замещения (С3), он в схеме с Э1.
const std::string kSelect =
    "SELECT e.id, e.room, e.kind, e.starts_at, e.ends_at, e.status, "
    "e.prev_room, e.prev_starts_at, e.prev_teacher_id, e.changed_at, "
    "e.teacher_id, "
    "d.code AS discipline_code, d.title AS discipline_title "
    "FROM schedule_events e JOIN disciplines d ON d.id = e.discipline_id ";

// Статусы «изменений» — фиксированный белый список из CHECK-ограничения
// 02 §4.1 (не пользовательский ввод): SQL-литералы, idx_schedule_group_status.
const std::string kChangedStatuses = "('moved','cancelled','substituted')";

Json::Value optString(const Row& r, const char* col)
{
    if (r[col].isNull())
        return Json::Value();
    return Json::Value(r[col].as<std::string>());
}

Json::Value optInt64(const Row& r, const char* col)
{
    if (r[col].isNull())
        return Json::Value();
    return Json::Value(r[col].as<int64_t>());
}

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
    // «было → стало» (С1/С3): null, когда переноса не было.
    e["prev_room"] = optString(r, "prev_room");
    e["prev_starts_at"] = optString(r, "prev_starts_at");
    e["prev_teacher_id"] = optInt64(r, "prev_teacher_id");
    e["changed_at"] = optString(r, "changed_at");
    // Э6а: профиль преподавателя для «Деталей пары» — /v1/people/{id} с
    // people-пространством (teacher_id + kPersonIdOffset, Р-15) считает FE.
    e["teacher_id"] = optInt64(r, "teacher_id");
    return e;
}

Json::Value eventsArray(const drogon::orm::Result& rows)
{
    Json::Value arr(Json::arrayValue);
    for (const auto& row : rows)
        arr.append(eventJson(row));
    return arr;
}

std::time_t dateToTime(const std::string& ymd)
{
    int y = 0, m = 0, d = 0;
    if (sscanf(ymd.c_str(), "%d-%d-%d", &y, &m, &d) != 3 || y < 1970 ||
        m < 1 || m > 12 || d < 1 || d > 31)
        return -1;
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_isdst = 0;
    return timegm(&tm);  // UTC без DST — арифметика суток линейна
}

std::string timeToDate(std::time_t t)
{
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}

std::string dayStartIso(const std::string& date)
{
    return date + "T00:00:00Z";
}

}  // namespace

std::string ScheduleService::todayUtc()
{
    return timeToDate(std::time(nullptr));
}

std::string ScheduleService::addDays(const std::string& date, int days)
{
    const std::time_t t = dateToTime(date);
    if (t < 0)
        return date;
    return timeToDate(t + static_cast<std::time_t>(days) * 86400);
}

std::string ScheduleService::weekStart(const std::string& date, int offset)
{
    const std::time_t t = dateToTime(date);
    if (t < 0)
        return date;
    std::tm tm{};
    gmtime_r(&t, &tm);
    const int daysSinceMonday = (tm.tm_wday + 6) % 7;  // 0 = понедельник
    const std::time_t monday =
        t - static_cast<std::time_t>(daysSinceMonday) * 86400 +
        static_cast<std::time_t>(offset) * 7 * 86400;
    return timeToDate(monday);
}

void ScheduleService::groupOfUser(
    DbClientPtr db,
    int64_t userId,
    std::function<void(GroupLookup, std::optional<int64_t>)> cb)
{
    *db << "SELECT group_id FROM profiles WHERE user_id = $1" << userId >>
        [cb](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                cb(GroupLookup::NotFound, std::nullopt);  // staff (Р-6)
                return;
            }
            cb(GroupLookup::Found, rows[0]["group_id"].as<int64_t>());
        } >>
        // Колбэк КОПИРУЕТСЯ в обе ветки цепочки (result/exception) —
        // стреляет ровно одна. `cb = std::move(cb)` в первой лямбде оставил
        // бы второй замыканию пустой std::function → std::bad_function_call
        // на error-пути БД (урок Э2 шаг 1, OwnershipGuard.cpp:56) — поймано
        // тестом fail-closed этого шага.
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "ScheduleService::groupOfUser(" << userId
                      << "): " << e.base().what();
            cb(GroupLookup::Error, std::nullopt);  // fail-closed → 500
        };
}

void ScheduleService::eventsOnRange(
    DbClientPtr db,
    int64_t groupId,
    const std::string& fromIso,
    const std::string& toIsoExclusive,
    std::function<void(bool ok, Json::Value events)> cb)
{
    // idx_schedule_group_time (group_id, starts_at): equality + диапазон +
    // ORDER BY — весь запрос по одному индексу (02 §4.2).
    const std::string sql =
        kSelect +
        "WHERE e.group_id = $1 AND e.starts_at >= $2 AND e.starts_at < $3 "
        "ORDER BY e.starts_at";
    *db << sql << groupId << fromIso << toIsoExclusive >>
        [cb](const drogon::orm::Result& rows) {
            cb(true, eventsArray(rows));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ScheduleService::eventsOnRange: " << e.base().what();
            cb(false, Json::Value());
        };
}

void ScheduleService::eventsOnDay(
    DbClientPtr db,
    int64_t groupId,
    const std::string& date,
    std::function<void(bool ok, Json::Value events)> cb)
{
    eventsOnRange(db,
                  groupId,
                  dayStartIso(date),
                  dayStartIso(addDays(date, 1)),
                  std::move(cb));
}

void ScheduleService::eventsByTeacher(
    DbClientPtr db,
    int64_t teacherId,
    std::function<void(bool ok, Json::Value events)> cb)
{
    // 5.11 «его занятия» (Э6а): фиксированная выборка по teacher_id
    // (не пользовательский SQL), порядок по времени. Индекса teacher_id в
    // схеме нет (02 §4.2 не предполагал выборку по преподавателю) — каталог
    // демо мал; при росте — частичный индекс (задача оптимизации, 02 §7.1).
    const std::string sql =
        kSelect + "WHERE e.teacher_id = $1 ORDER BY e.starts_at";
    // Ловушка Э6 шага 4 (задокументирована в журнале): неквалифицированный
    // `Result` в лямбдах member-функции резолвится во ВЛОЖЕННЫЙ
    // ScheduleService::Result — только полная квалификация.
    *db << sql << teacherId >>
        [cb](const drogon::orm::Result& rows) {
            cb(true, eventsArray(rows));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ScheduleService::eventsByTeacher: "
                      << e.base().what();
            cb(false, Json::Value());
        };
}

void ScheduleService::recentChanges(
    DbClientPtr db,
    int64_t groupId,
    int days,
    const std::string& today,
    std::function<void(bool ok, Json::Value events)> cb)
{
    // idx_schedule_group_status (group_id, status, starts_at): equality +
    // IN по статусу + диапазон дат (02 §4.2, «Изменения недели» С3).
    const std::string sql =
        kSelect +
        "WHERE e.group_id = $1 AND e.status IN " + kChangedStatuses +
        " AND e.starts_at >= $2 AND e.starts_at < $3 ORDER BY e.starts_at";
    // Окно — последние days календарных суток, заканчивающиеся today:
    // [today-(days-1), today+1). Изменение будущей даты показывается в день
    // её наступления; оперативное оповещение о нём — push (Э6).
    const std::string fromIso = dayStartIso(addDays(today, -(days - 1)));
    const std::string toIso = dayStartIso(addDays(today, 1));
    *db << sql << groupId << fromIso << toIso >>
        [cb](const drogon::orm::Result& rows) {
            cb(true, eventsArray(rows));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ScheduleService::recentChanges: " << e.base().what();
            cb(false, Json::Value());
        };
}

void ScheduleService::today(DbClientPtr db,
                            int64_t userId,
                            const std::string& today,
                            ResultCb cb)
{
    groupOfUser(
        db,
        userId,
        [db, today, cb = std::move(cb)](GroupLookup lookup,
                                        std::optional<int64_t> groupId) mutable {
            if (lookup == GroupLookup::Error)
            {
                cb(Result{false, Json::Value()});  // 500 без деталей
                return;
            }
            if (lookup == GroupLookup::NotFound)  // staff: группа не определена
            {
                Json::Value body;
                body["date"] = today;
                body["events"] = Json::Value(Json::arrayValue);
                cb(Result{true, std::move(body)});
                return;
            }
            eventsOnDay(
                db,
                *groupId,
                today,
                [today, cb = std::move(cb)](bool ok,
                                            Json::Value events) mutable {
                    if (!ok)
                    {
                        cb(Result{false, Json::Value()});
                        return;
                    }
                    Json::Value body;
                    body["date"] = today;
                    body["events"] = std::move(events);
                    cb(Result{true, std::move(body)});
                });
        });
}

void ScheduleService::week(DbClientPtr db,
                           int64_t userId,
                           int offset,
                           ScheduleCache* cache,
                           const std::string& today,
                           ResultCb cb)
{
    const std::string from = weekStart(today, offset);
    const std::string to = addDays(from, 6);
    groupOfUser(
        db,
        userId,
        [db, offset, cache, from, to, cb = std::move(cb)](
            GroupLookup lookup, std::optional<int64_t> groupId) mutable {
            if (lookup == GroupLookup::Error)
            {
                cb(Result{false, Json::Value()});
                return;
            }
            // weekBody захватывает КОПИИ (не ссылки на члены внешнего
            // замыкания): внутренняя лямбда переживает внешний колбэк —
            // внешнее замыкание уничтожается сразу после вызова.
            const auto weekBody = [offset, from, to](Json::Value events) {
                Json::Value body;
                body["offset"] = offset;
                body["from"] = from;
                body["to"] = to;
                body["events"] = std::move(events);
                return body;
            };
            if (lookup == GroupLookup::NotFound)  // staff: пустая неделя
            {
                cb(Result{true, weekBody(Json::Value(Json::arrayValue))});
                return;
            }
            const int64_t g = *groupId;
            WeekModel cached;
            if (cache != nullptr && cache->get(g, cached) &&
                cached.from == from && cached.to == to)
            {
                // Попадание (проверка from/to страхует смену недели в кэше).
                cb(Result{true, weekBody(std::move(cached.events))});
                return;
            }
            const std::string fromIso = dayStartIso(from);
            const std::string toIso = dayStartIso(addDays(from, 7));
            eventsOnRange(
                db,
                g,
                fromIso,
                toIso,
                [cache, from, to, g, weekBody, cb = std::move(cb)](
                    bool ok, Json::Value events) mutable {
                    if (!ok)
                    {
                        cb(Result{false, Json::Value()});
                        return;
                    }
                    if (cache != nullptr)
                    {
                        WeekModel m;
                        m.groupId = g;
                        m.from = from;
                        m.to = to;
                        m.events = events;  // копия: events уйдёт в ответ
                        cache->put(g, std::move(m));
                    }
                    cb(Result{true, weekBody(std::move(events))});
                });
        });
}

void ScheduleService::changeList(DbClientPtr db,
                                 int64_t userId,
                                 int days,
                                 const std::string& today,
                                 ResultCb cb)
{
    groupOfUser(
        db,
        userId,
        [db, days, today, cb = std::move(cb)](GroupLookup lookup,
                                              std::optional<int64_t> groupId) mutable {
            // Копия days по значению — внутренняя лямбда переживает
            // внешний колбэк (см. комментарий в week()).
            const auto changesBody = [days](Json::Value events) {
                Json::Value body;
                body["days"] = days;
                body["changes"] = std::move(events);
                return body;
            };
            if (lookup == GroupLookup::Error)
            {
                cb(Result{false, Json::Value()});
                return;
            }
            if (lookup == GroupLookup::NotFound)  // staff: изменений нет
            {
                cb(Result{true, changesBody(Json::Value(Json::arrayValue))});
                return;
            }
            recentChanges(
                db,
                *groupId,
                days,
                today,
                [changesBody, cb = std::move(cb)](bool ok,
                                                  Json::Value events) mutable {
                    if (!ok)
                    {
                        cb(Result{false, Json::Value()});
                        return;
                    }
                    cb(Result{true, changesBody(std::move(events))});
                });
        });
}

}  // namespace para
