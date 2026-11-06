#include "services/RoomsService.h"

#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// ВАЖНО: внутри namespace para неквалифицированный `Result` — это
// para::RoomsService::Result (член класса), а НЕ drogon::orm::Result:
// типы строк выборок в лямбдах ниже пишутся полностью —
// `const drogon::orm::Result&` (как в ScheduleService.cpp).

// Окно сетки слотов — учебный день UTC 08:00–22:00 (см. шапку заголовка).
constexpr int kFirstSlotHour = 8;
constexpr int kLastSlotHour = 22;  // полуинтервал: последний слот 21:00–22:00

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

// Календарный день date — полуинтервал [T00:00:00Z, на+1 T00:00:00Z).
std::string dayStartIso(const std::string& date)
{
    return date + "T00:00:00Z";
}

std::string dayEndIso(const std::string& date)
{
    return timeToDate(dateToTime(date) + 86400) + "T00:00:00Z";
}

// Метка сетки слота: date + T«час»:00:00Z (формат строк фикстур/Schedule).
std::string hourIso(const std::string& date, int hour)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "T%02d:00:00Z", hour);
    return date + buf;
}

Json::Value roomJson(const drogon::orm::Row& r)
{
    Json::Value room(Json::objectValue);
    room["id"] = r["id"].as<int64_t>();
    room["name"] = r["name"].as<std::string>();
    room["building"] = r["building"].as<std::string>();
    room["capacity"] = r["capacity"].as<int64_t>();
    room["features"] = r["features"].as<std::string>();
    return room;
}

}  // namespace

std::string RoomsService::todayUtc()
{
    return timeToDate(std::time(nullptr));
}

bool RoomsService::validDate(const std::string& date)
{
    const std::time_t t = dateToTime(date);
    if (t < 0)
        return false;
    // Календарность: timegm нормализует 2026-02-30 → 02.03 — обратная
    // отрисовка должна совпасть байт-в-байт, иначе даты не было.
    return timeToDate(t) == date;
}

void RoomsService::catalog(DbClientPtr db, ResultCb cb)
{
    *db << "SELECT id, name, building, capacity, features FROM rooms "
           "ORDER BY id" >>
        [cb](const drogon::orm::Result& rows) mutable {
            Json::Value body;
            Json::Value arr(Json::arrayValue);
            for (const auto& row : rows)
                arr.append(roomJson(row));
            body["rooms"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        // Колбэк КОПИРУЕТСЯ в обе ветки цепочки (result/exception) —
        // стреляет ровно одна (урок Э2 шаг 1: move оставил бы второму
        // замыканию пустой std::function → bad_function_call на error-пути).
        [cb](const DrogonDbException& e) mutable {
            LOG_ERROR << "RoomsService::catalog: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void RoomsService::roomById(
    DbClientPtr db,
    int64_t roomId,
    std::function<void(Lookup, Json::Value)> cb)
{
    *db << "SELECT id, name, building FROM rooms WHERE id = $1" << roomId >>
        [cb](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                cb(Lookup::NotFound, Json::Value());
                return;
            }
            Json::Value room(Json::objectValue);
            room["id"] = rows[0]["id"].as<int64_t>();
            room["name"] = rows[0]["name"].as<std::string>();
            room["building"] = rows[0]["building"].as<std::string>();
            cb(Lookup::Found, std::move(room));
        } >>
        [cb, roomId](const DrogonDbException& e) mutable {
            LOG_ERROR << "RoomsService::roomById(" << roomId
                      << "): " << e.base().what();
            cb(Lookup::Error, Json::Value());  // fail-closed → 500
        };
}

void RoomsService::slots(DbClientPtr db,
                         int64_t roomId,
                         int64_t userId,
                         const std::string& date,
                         std::function<void(SlotsResult)> cb)
{
    // Fail-closed: сетка по невалидной дате невозможна — «весь день
    // свободен» врал бы; контроллер валидирует ?date= до вызова (400),
    // здесь та же проверка страхует прямой вызов.
    if (!validDate(date))
    {
        cb(SlotsResult{SlotsStatus::Error, Json::Value()});
        return;
    }
    const std::string fromIso = dayStartIso(date);
    const std::string toIso = dayEndIso(date);

    roomById(
        db,
        roomId,
        [db, roomId, userId, date, fromIso, toIso, cb = std::move(cb)](
            Lookup lookup, Json::Value room) mutable {
            if (lookup == Lookup::Error)
            {
                cb(SlotsResult{SlotsStatus::Error, Json::Value()});  // 500
                return;
            }
            if (lookup == Lookup::NotFound)
            {
                cb(SlotsResult{SlotsStatus::NotFound, Json::Value()});  // 404
                return;
            }
            // Активные брони дня (пересекающие [from, to)). Статус —
            // SQL-литерал CHECK 0005, НЕ пользовательский ввод; параметры
            // в тексте по порядку $1→$3 = порядку биндинга (урок Э5 шаг 2:
            // $N индексируется по первому вхождению). status='booked'
            // имплицирует условие частичного индекса idx_bookings_active
            // ((room_id, starts_at) WHERE status='booked'): equality +
            // диапазон + ORDER BY — один индекс, без TEMP B-TREE
            // (EXPLAIN QUERY PLAN проверен). Блокировки ('blocked') —
            // шаг 3 Э5а, в сетке шага 1 не участвуют (см. шапку .h).
            *db << "SELECT user_id, starts_at, ends_at FROM room_bookings "
                   "WHERE room_id = $1 AND status = 'booked' AND starts_at "
                   "< $2 AND ends_at > $3 ORDER BY starts_at"
                << roomId << toIso << fromIso >>
                [room = std::move(room), date, userId, cb = std::move(cb)](
                    const drogon::orm::Result& rows) mutable {
                    // Строки дня → интервалы: слот занят, если есть
                    // пересечение (row.starts_at < slotEnd && row.ends_at >
                    // slotStart) — точное перекрытие, не совпадение начала;
                    // «mine» — строка текущей сессии.
                    struct Occupied
                    {
                        std::string startsAt;
                        std::string endsAt;
                        bool mine = false;
                    };
                    std::vector<Occupied> occupied;
                    occupied.reserve(rows.size());
                    for (const auto& row : rows)
                        occupied.push_back({row["starts_at"].as<std::string>(),
                                            row["ends_at"].as<std::string>(),
                                            row["user_id"].as<int64_t>() ==
                                                userId});

                    Json::Value slotsArr(Json::arrayValue);
                    for (int hour = kFirstSlotHour; hour < kLastSlotHour;
                         ++hour)
                    {
                        const std::string slotStart = hourIso(date, hour);
                        const std::string slotEnd = hourIso(date, hour + 1);
                        Json::Value slot(Json::objectValue);
                        slot["starts_at"] = slotStart;
                        slot["ends_at"] = slotEnd;
                        slot["state"] = "free";
                        for (const auto& o : occupied)
                        {
                            if (o.startsAt < slotEnd && o.endsAt > slotStart)
                            {
                                slot["state"] = o.mine ? "mine" : "booked";
                                break;
                            }
                        }
                        slotsArr.append(std::move(slot));
                    }

                    // Тело 02 §5.2: шапка аудитории (для sheet-заголовка
                    // «Ауд. 1207 · Корпус А») + сетка дня.
                    Json::Value body(Json::objectValue);
                    body["room_id"] = room["id"];
                    body["room"] = room["name"];
                    body["building"] = room["building"];
                    body["date"] = date;
                    body["slots"] = std::move(slotsArr);
                    cb(SlotsResult{SlotsStatus::Ok, std::move(body)});
                } >>
                [cb](const DrogonDbException& e) mutable {
                    LOG_ERROR << "RoomsService::slots(booking query): "
                              << e.base().what();
                    cb(SlotsResult{SlotsStatus::Error,
                                   Json::Value()});  // fail-closed → 500
                };
        });
}

}  // namespace para
