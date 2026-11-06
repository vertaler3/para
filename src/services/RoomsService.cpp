#include "services/RoomsService.h"

#include <cctype>
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
                // Колбэки-ветки копируют cb (стреляет ровно одна): move в
                // success-ветке оставил бы error-ветке пустой std::function
                // → bad_function_call на error-пути (урок Э2 шаг 1).
                [room = std::move(room), date, userId, cb](
                    const drogon::orm::Result& rows) {
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

// --- Брони (Э5а шаг 2) -----------------------------------------------------

namespace {

// Строгое «YYYY-MM-DDTHH:MM:SSZ» (формат фикстур и SessionService::utcNow).
// Формат обязан быть единым: все сравнения меток в этом сервисе (пересечение,
// «тот же слот», upcoming) — лексикографические по тексту; вход валидируется
// строго, чтобы сравнение не соврало (2026-11-13T10:00 vs 2026-11-13 10:00).
bool isIsoZ(const std::string& s)
{
    if (s.size() != 20 || s[4] != '-' || s[7] != '-' || s[10] != 'T' ||
        s[13] != ':' || s[16] != ':' || s[19] != 'Z')
        return false;
    const auto digits = [&s](std::size_t i) {
        return std::isdigit(static_cast<unsigned char>(s[i])) != 0;
    };
    for (std::size_t i = 0; i < 4; ++i)
        if (!digits(i))
            return false;
    for (const std::size_t i : {5u, 8u, 11u, 14u, 17u})
        if (!digits(i) || !digits(i + 1))
            return false;
    const int hour = std::stoi(s.substr(11, 2));
    const int minute = std::stoi(s.substr(14, 2));
    const int second = std::stoi(s.substr(17, 2));
    if (hour > 23 || minute > 59 || second > 59)
        return false;
    return RoomsService::validDate(s.substr(0, 10));  // календарность даты
}

}  // namespace

void RoomsService::create(DbClientPtr db,
                          int64_t roomId,
                          int64_t userId,
                          const std::string& startsAt,
                          const std::string& endsAt,
                          std::function<void(CreateResult)> cb)
{
    if (!isIsoZ(startsAt) || !isIsoZ(endsAt) || endsAt <= startsAt)
    {
        cb(CreateResult{CreateStatus::InvalidBody, Json::Value()});
        return;
    }
    roomById(
        db,
        roomId,
        [db, roomId, userId, startsAt, endsAt, cb](  // копия: см. урок ниже
            Lookup lookup, Json::Value) {
            if (lookup == Lookup::Error)
            {
                cb(CreateResult{CreateStatus::Error, Json::Value()});
                return;
            }
            if (lookup == Lookup::NotFound)
            {
                cb(CreateResult{CreateStatus::NotFound, Json::Value()});
                return;
            }
            // Серверная проверка пересечений — только активные брони
            // (status='booked', idx_bookings_active, см. шапку slots);
            // параметры в тексте по порядку $1→$3 (урок Э5 шаг 2).
            *db << "SELECT starts_at FROM room_bookings WHERE room_id = $1 "
                   "AND status = 'booked' AND starts_at < $2 AND ends_at > $3"
                << roomId << endsAt << startsAt >>
                // Урок Э2 шаг 1 (двойной move в цепочке >>): каждая ветка
                // копирует cb — move в success-ветке оставил бы error-ветке
                // пустой std::function → bad_function_call на error-пути
                // (поймано тестом «исключение БД на INSERT» этого шага).
                [db, roomId, userId, startsAt, endsAt, cb](
                    const drogon::orm::Result& rows) {
                    // «Тот же слот» — пересекающаяся бронь с тем же starts_at
                    // (уникальный ключ idx_bookings_active): двойная бронь —
                    // 400 (гайд BE §8). Частичное перекрытие — 409 «только
                    // что заняли» (случай FE-сетки, 03 §4.9.5).
                    for (const auto& row : rows)
                    {
                        if (row["starts_at"].as<std::string>() == startsAt)
                        {
                            cb(CreateResult{CreateStatus::SlotTaken,
                                            Json::Value()});
                            return;
                        }
                    }
                    if (rows.size() > 0)
                    {
                        cb(CreateResult{CreateStatus::Conflict, Json::Value()});
                        return;
                    }
                    // INSERT: статус — литерал CHECK 0005; порядок $1→$4 =
                    // порядку биндинга. Исключение БД здесь — нарушение
                    // ограничения в гонке (UNIQUE idx_bookings_active:
                    // серверная проверка обеих параллельных POST прошла до
                    // INSERT'ов; прочие предусловия проверены выше) —
                    // маппится в SlotTaken/400, НЕ в 500 (постановка шага 2).
                    *db << "INSERT INTO room_bookings (room_id, user_id, "
                           "starts_at, ends_at, status) VALUES "
                           "($1, $2, $3, $4, 'booked')"
                        << roomId << userId << startsAt << endsAt >>
                        [db, roomId, startsAt, endsAt, cb](
                            const drogon::orm::Result&) {
                            // id вставленной строки: активная бронь
                            // (room_id, starts_at) единственна по
                            // idx_bookings_active — детерминированный
                            // SELECT по ключу вместо last_insert_rowid
                            // (пул соединений).
                            *db << "SELECT id FROM room_bookings WHERE "
                                   "room_id = $1 AND starts_at = $2 AND "
                                   "status = 'booked'"
                                << roomId << startsAt >>
                                [roomId, startsAt, endsAt, cb](
                                    const drogon::orm::Result& rows) {
                                    Json::Value body(Json::objectValue);
                                    if (!rows.empty())
                                        body["id"] =
                                            rows[0]["id"].as<int64_t>();
                                    body["room_id"] = roomId;
                                    body["starts_at"] = startsAt;
                                    body["ends_at"] = endsAt;
                                    body["status"] = "booked";
                                    cb(CreateResult{CreateStatus::Created,
                                                    std::move(body)});
                                } >>
                                [cb](const DrogonDbException& e) {
                                    LOG_ERROR << "RoomsService::create(select "
                                                 "id): "
                                              << e.base().what();
                                    cb(CreateResult{CreateStatus::Error,
                                                    Json::Value()});
                                };
                        } >>
                        [cb](const DrogonDbException& e) {
                            LOG_ERROR << "RoomsService::create(insert): "
                                      << e.base().what();
                            cb(CreateResult{CreateStatus::SlotTaken,
                                            Json::Value()});  // гонка → 400
                        };
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "RoomsService::create(overlap check): "
                              << e.base().what();
                    cb(CreateResult{CreateStatus::Error, Json::Value()});
                };
        });
}

void RoomsService::myBookings(DbClientPtr db,
                              int64_t userId,
                              bool upcoming,
                              const std::string& nowIso,
                              ResultCb cb)
{
    // idx_bookings_user (user_id, starts_at): equality + ORDER BY (02 §4.2
    // «мои брони»); JOIN rooms — имя/корпус для строки списка. upcoming —
    // флаг-константа ветвления SQL (фильтры-or-empty — паттерн
    // ApplicationsService::list), значения строк в SQL не подставляются.
    const std::string kBase =
        "SELECT b.id, b.room_id, b.starts_at, b.ends_at, b.status, "
        "r.name AS room, r.building FROM room_bookings b "
        "JOIN rooms r ON r.id = b.room_id "
        "WHERE b.user_id = $1 AND b.status = 'booked'";
    const std::string kUpcomingSql =
        kBase + " AND b.ends_at >= $2 ORDER BY b.starts_at";
    const std::string kAllSql = kBase + " ORDER BY b.starts_at";

    auto onRows = [cb](const drogon::orm::Result& rows) {
        Json::Value arr(Json::arrayValue);
        for (const auto& r : rows)
        {
            Json::Value b(Json::objectValue);
            b["id"] = r["id"].as<int64_t>();
            b["room_id"] = r["room_id"].as<int64_t>();
            b["room"] = r["room"].as<std::string>();
            b["building"] = r["building"].as<std::string>();
            b["starts_at"] = r["starts_at"].as<std::string>();
            b["ends_at"] = r["ends_at"].as<std::string>();
            b["status"] = r["status"].as<std::string>();
            arr.append(std::move(b));
        }
        Json::Value body;
        body["bookings"] = std::move(arr);
        cb(Result{true, std::move(body)});
    };
    auto onError = [cb, userId](const DrogonDbException& e) {
        LOG_ERROR << "RoomsService::myBookings(" << userId
                  << "): " << e.base().what();
        cb(Result{false, Json::Value()});  // fail-closed → 500
    };

    if (upcoming)
        *db << kUpcomingSql << userId << nowIso >> std::move(onRows) >>
            std::move(onError);
    else
        *db << kAllSql << userId >> std::move(onRows) >> std::move(onError);
}

void RoomsService::cancelById(DbClientPtr db,
                              int64_t bookingId,
                              std::function<void(CancelStatus)> cb)
{
    // Условный UPDATE booked→cancelled: 1 строка — отмена; 0 строк — не
    // активная (уже cancelled/blocked) либо строки нет (гонка после Own —
    // различаются SELECT'ом ниже). Владение проверил guard контроллера —
    // здесь по PK без user_id-условия (постановка Э4 шаг 2).
    *db << "UPDATE room_bookings SET status = 'cancelled' "
           "WHERE id = $1 AND status = 'booked'"
        << bookingId >>
        // Ветки копируют cb (стреляет одна) — урок Э2 шаг 1: move в
        // success-ветке оставил бы error-ветке пустой std::function.
        [db, bookingId, cb](const drogon::orm::Result& result) {
            if (result.affectedRows() > 0)
            {
                cb(CancelStatus::Canceled);  // → 204
                return;
            }
            *db << "SELECT status FROM room_bookings WHERE id = $1"
                << bookingId >>
                [cb](const drogon::orm::Result& rows) {
                    if (rows.empty())
                        cb(CancelStatus::NotFound);  // → 404 (не маскируется)
                    else
                        cb(CancelStatus::NotActive);  // → 409 (не booked)
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "RoomsService::cancelById(select): "
                              << e.base().what();
                    cb(CancelStatus::Error);
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "RoomsService::cancelById(update): "
                      << e.base().what();
            cb(CancelStatus::Error);  // fail-closed → 500
        };
}

}  // namespace para
