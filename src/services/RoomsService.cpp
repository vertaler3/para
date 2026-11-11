#include "services/RoomsService.h"

#include <atomic>
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

void RoomsService::catalog(DbClientPtr db,
                           const std::optional<int64_t>& minCapacity,
                           ResultCb cb)
{
    // Фильтры-or-empty (паттерн ApplicationsService::list): две полные
    // SQL-строки, значение фильтра — параметром ($1), не конкатенация.
    const std::string kAllSql =
        "SELECT id, name, building, capacity, features FROM rooms "
        "ORDER BY id";
    const std::string kMinCapSql =
        "SELECT id, name, building, capacity, features FROM rooms "
        "WHERE capacity >= $1 ORDER BY id";

    auto onRows = [cb](const drogon::orm::Result& rows) {
        Json::Value body;
        Json::Value arr(Json::arrayValue);
        for (const auto& row : rows)
            arr.append(roomJson(row));
        body["rooms"] = std::move(arr);
        cb(Result{true, std::move(body)});
    };
    auto onError = [cb](const DrogonDbException& e) {
        LOG_ERROR << "RoomsService::catalog: " << e.base().what();
        cb(Result{false, Json::Value()});  // fail-closed → 500
    };

    if (minCapacity.has_value())
        *db << kMinCapSql << *minCapacity >> std::move(onRows) >>
            std::move(onError);
    else
        *db << kAllSql >> std::move(onRows) >> std::move(onError);
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
            // Строки дня, занимающие слоты (шаг 3): UNION ALL двух
            // статус-веток — 'booked' (idx_bookings_active, EXPLAIN SEARCH)
            // и 'blocked' (блокировки деканата, единичные строки). Статусы —
            // SQL-литералы CHECK 0005; параметры в тексте по порядку
            // $1→$6 = порядку биндинга (урок Э5 шаг 2: $N — по первому
            // вхождению). Сортировка не нужна —occupied собирается в C++.
            *db << "SELECT user_id, starts_at, ends_at, status FROM "
                   "room_bookings WHERE room_id = $1 AND status = 'booked' "
                   "AND starts_at < $2 AND ends_at > $3 "
                   "UNION ALL "
                   "SELECT user_id, starts_at, ends_at, status FROM "
                   "room_bookings WHERE room_id = $4 AND status = 'blocked' "
                   "AND starts_at < $5 AND ends_at > $6"
                << roomId << toIso << fromIso << roomId << toIso << fromIso >>
                // Колбэки-ветки копируют cb (стреляет ровно одна): move в
                // success-ветке оставил бы error-ветке пустой std::function
                // → bad_function_call на error-пути (урок Э2 шаг 1).
                [room = std::move(room), date, userId, cb](
                    const drogon::orm::Result& rows) {
                    // Строки дня → интервалы: слот занят, если есть
                    // пересечение (row.starts_at < slotEnd && row.ends_at >
                    // slotStart) — точное перекрытие, не совпадение начала;
                    // «mine» — только своя АКТИВНАЯ бронь (status='booked' и
                    // user_id сессии); всё остальное пересечение — «занято»
                    // (чужая бронь или блокировка деканата, шаг 3).
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
                                            row["status"].as<std::string>() ==
                                                    "booked" &&
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
            // ГОНКА БРОНЕЙ (находка Э5а «create: SELECT и INSERT не в одной
            // транзакции», закрыта кодом в шаге 7 Э8): раньше проверка
            // пересечения и вставка были РАЗДЕЛЬНЫМИ командами — параллельные
            // POST с РАЗНЫМИ starts_at, пересекающимися по времени, оба
            // проходили (оба SELECT ничего не видели; UNIQUE
            // idx_bookings_active ловит только одинаковый starts_at). Теперь
            // проверка и вставка — ОДИН атомарный guarded INSERT..SELECT:
            // предикат NOT EXISTS исполняется движком под тем же блоком
            // записи, что вставка (SQLite: инструкция атомарна, писатели
            // сериализуются, busy_timeout ждёт), поэтому второй параллельный
            // POST видит вставленную строку и не вставляет
            // (affectedRows = 0 → классификация ниже).
            // Порядок $N в тексте = порядку биндинга (урок Э5 шаг 2): первые
            // вхождения $1→$4 в SELECT-списке, дальше переиспользование.
            // Проверка covers обе линии прежних SELECT'ов: активные брони
            // ('booked', idx_bookings_active) и блокировки деканата
            // ('blocked' — единственная линия проверки, UNIQUE её не
            // закрывает).
            *db << "INSERT INTO room_bookings (room_id, user_id, starts_at, "
                   "ends_at, status) SELECT $1, $2, $3, $4, 'booked' WHERE "
                   "NOT EXISTS (SELECT 1 FROM room_bookings WHERE room_id = "
                   "$1 AND status IN ('booked', 'blocked') AND starts_at < "
                   "$4 AND ends_at > $3)"
                << roomId << userId << startsAt << endsAt >>
                // Урок Э2 шаг 1 (двойной move в цепочке >>): каждая ветка
                // копирует cb — move в success-ветке оставил бы error-ветке
                // пустой std::function → bad_function_call на error-пути.
                [db, roomId, startsAt, endsAt, cb](
                    const drogon::orm::Result& inserted) {
                    if (inserted.affectedRows() == 0)
                    {
                        // Слот занят (пересечение с 'booked' или 'blocked').
                        // Классификация ПОСЛЕ факта — прежними двумя
                        // SELECT'ами: «тот же starts_at» (UNIQUE
                        // idx_bookings_active) → SlotTaken/400 «двойная
                        // бронь» (гайд BE §8); частичное перекрытие →
                        // Conflict/409 «только что заняли» (03 §4.9.5);
                        // блокировка деканата → Blocked. Если классификация
                        // ничего не находит (гонка с отменой брони между
                        // INSERT и классификацией) — консервативный
                        // Conflict: слот в момент попытки был занят.
                        classifyTaken(db,
                                      roomId,
                                      startsAt,
                                      endsAt,
                                      std::move(cb));
                        return;
                    }
                    // id вставленной строки: активная бронь
                    // (room_id, starts_at) единственна по
                    // idx_bookings_active — детерминированный
                    // SELECT по ключу вместо last_insert_rowid
                    // (пул соединений).
                    *db << "SELECT id FROM room_bookings WHERE room_id = $1 "
                           "AND starts_at = $2 AND status = 'booked'"
                        << roomId << startsAt >>
                        [roomId, startsAt, endsAt, cb](
                            const drogon::orm::Result& rows) {
                            Json::Value body(Json::objectValue);
                            if (!rows.empty())
                                body["id"] = rows[0]["id"].as<int64_t>();
                            body["room_id"] = roomId;
                            body["starts_at"] = startsAt;
                            body["ends_at"] = endsAt;
                            body["status"] = "booked";
                            cb(CreateResult{CreateStatus::Created,
                                            std::move(body)});
                        } >>
                        [cb](const DrogonDbException& e) {
                            LOG_ERROR << "RoomsService::create(select id): "
                                      << e.base().what();
                            cb(CreateResult{CreateStatus::Error,
                                            Json::Value()});
                        };
                } >>
                [cb](const DrogonDbException& e) {
                    // Исключение БД здесь — нарушение ограничения в гонке
                    // (UNIQUE idx_bookings_active) — маппится в
                    // SlotTaken/400, НЕ в 500 (постановка Э5а шага 2).
                    LOG_ERROR << "RoomsService::create(insert): "
                              << e.base().what();
                    cb(CreateResult{CreateStatus::SlotTaken, Json::Value()});
                };
        });
}

void RoomsService::classifyTaken(DbClientPtr db,
                                 int64_t roomId,
                                 const std::string& startsAt,
                                 const std::string& endsAt,
                                 std::function<void(CreateResult)> cb)
{
    // Классификация занятого слота для ветки affectedRows = 0 (см. create):
    // те же предикаты, что раньше стояли ДО вставки. cb копируется в каждую
    // ветку (урок Э2).
    *db << "SELECT starts_at FROM room_bookings WHERE room_id = $1 "
           "AND status = 'booked' AND starts_at < $2 AND ends_at > $3"
        << roomId << endsAt << startsAt >>
        [db, roomId, startsAt, endsAt, cb](const drogon::orm::Result& rows) {
            // «Тот же слот» — пересекающаяся бронь с тем же starts_at
            // (уникальный ключ idx_bookings_active): двойная бронь —
            // 400. Частичное перекрытие — 409.
            for (const auto& row : rows)
            {
                if (row["starts_at"].as<std::string>() == startsAt)
                {
                    cb(CreateResult{CreateStatus::SlotTaken, Json::Value()});
                    return;
                }
            }
            if (rows.size() > 0)
            {
                cb(CreateResult{CreateStatus::Conflict, Json::Value()});
                return;
            }
            *db << "SELECT 1 FROM room_bookings WHERE room_id = $1 "
                   "AND status = 'blocked' AND starts_at < $2 AND "
                   "ends_at > $3"
                << roomId << endsAt << startsAt >>
                [cb](const drogon::orm::Result& blockedRows) {
                    cb(CreateResult{blockedRows.size() > 0
                                        ? CreateStatus::Blocked
                                        : CreateStatus::Conflict,
                                    Json::Value()});
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "RoomsService::classifyTaken(blocked): "
                              << e.base().what();
                    cb(CreateResult{CreateStatus::Error, Json::Value()});
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "RoomsService::classifyTaken(booked): "
                      << e.base().what();
            cb(CreateResult{CreateStatus::Error, Json::Value()});
        };
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

// --- Блокировка деканатом (Э5а шаг 3) --------------------------------------

void RoomsService::block(DbClientPtr db,
                         int64_t roomId,
                         int64_t staffUserId,
                         const std::string& startsAt,
                         const std::string& endsAt,
                         const std::string& reason,
                         std::function<void(BlockResult)> cb)
{
    // Тело валидирует сервис (как create: строгий ISO «…Z», ends > starts);
    // reason проверяет контроллер (непустая строка) — он же кладёт его в
    // текст уведомления.
    if (!isIsoZ(startsAt) || !isIsoZ(endsAt) || endsAt <= startsAt)
    {
        cb(BlockResult{BlockStatus::InvalidBody, Json::Value()});
        return;
    }
    roomById(
        db,
        roomId,
        [db, roomId, staffUserId, startsAt, endsAt, reason, cb](
            Lookup lookup, Json::Value room) {
            if (lookup == Lookup::Error)
            {
                cb(BlockResult{BlockStatus::Error, Json::Value()});
                return;
            }
            if (lookup == Lookup::NotFound)
            {
                cb(BlockResult{BlockStatus::NotFound, Json::Value()});
                return;
            }
            const std::string roomName = room["name"].asString();

            // Транзакция (как AppWorkflow::apply): снятие пересекающихся
            // активных броней + уведомления авторам + INSERT блокировки —
            // атомарно; ошибка любого шага → rollback + Error (fail-closed,
            // 02 §5.1: 500 без деталей). Результат отдаёт commit-callback —
            // ответ клиенту не опережает фиксацию (урок E2E этого шага:
            // fire до коммита читался sqlite-SELECT'ом как «бронь не
            // снята»).
            auto once = std::make_shared<std::atomic_bool>(false);
            auto fire = [cb, once](BlockStatus status, Json::Value body) {
                bool expected = false;
                if (!once->compare_exchange_strong(expected, true))
                    return;
                if (status == BlockStatus::Error)
                {
                    LOG_WARN << "RoomsService::block: ошибка БД, транзакция "
                                "откатывается";
                }
                cb(BlockResult{status, std::move(body)});
            };

            db->newTransactionAsync(
                [roomId, staffUserId, startsAt, endsAt, reason, roomName,
                 fire](const std::shared_ptr<Transaction>& tx) {
                    if (!tx)
                    {
                        LOG_ERROR << "RoomsService::block: транзакция "
                                     "недоступна (timeout)";
                        fire(BlockStatus::Error, Json::Value());
                        return;
                    }
                    // Линейная цепочка из 2 SQL + INSERT + rowid-SELECT,
                    // БЕЗ рекурсивного шага: самоссылающийся shared_ptr на
                    // функцию держит замыкание → tx не освобождает
                    // соединение после «коммита» (поймано зависанием E2E
                    // на первом же запросе после блокировки). Порядок важен:
                    //   1) уведомления вставляются SELECT'ом по ЕЩЁ активным
                    //      броням — по одной на каждую снятую строку (текст
                    //      общий; словарь type CHECK 0004 — 'schedule':
                    //      бронь 5.10 живёт в шапке расписания 03 §4.9.5;
                    //      deep_link — экран «Мои брони»);
                    //   2) UPDATE снимает те же строки (status='cancelled',
                    //      строки сохраняются — история); affectedRows —
                    //      счётчик снятий для тела ответа;
                    //   3) INSERT блокировки status='blocked'
                    //      (user_id = staff);
                    //   4) id — last_insert_rowid() на соединении
                    //      транзакции (как AppWorkflow::apply).
                    // Параметры в тексте по порядку $1→$N = порядку биндинга
                    // (урок Э5 шаг 2, в т.ч. для INSERT..SELECT). Колбэки
                    // копируют захваты (урок Э2: стреляет ровно одна ветка).
                    const auto cancelledCount = std::make_shared<int64_t>(0);
                    const auto blockId = std::make_shared<int64_t>(0);
                    const std::string kNotifySql =
                        "INSERT INTO notifications (user_id, type, title, "
                        "body, deep_link) SELECT user_id, 'schedule', $1, "
                        "$2, $3 FROM room_bookings WHERE room_id = $4 AND "
                        "status = 'booked' AND starts_at < $5 AND ends_at "
                        "> $6";
                    const std::string kCancelSql =
                        "UPDATE room_bookings SET status = 'cancelled' "
                        "WHERE room_id = $1 AND status = 'booked' AND "
                        "starts_at < $2 AND ends_at > $3";
                    const std::string kInsertBlockSql =
                        "INSERT INTO room_bookings (room_id, user_id, "
                        "starts_at, ends_at, status) VALUES ($1, $2, $3, "
                        "$4, 'blocked')";
                    const std::string title = "Бронь снята деканатом";
                    const std::string bodyText = "Бронь аудитории " +
                        roomName + " (" + startsAt + "–" + endsAt +
                        ") снята: " + reason;
                    const std::string deepLink = "/room-bookings";

                    *tx << kNotifySql << title << bodyText << deepLink
                        << roomId << endsAt << startsAt >>
                        // Промежуточные лямбды захватывают и то, что нужно
                        // ВЛОЖЕННЫМ (kInsertBlockSql/blockId — локалы
                        // функции: без этого вложенный захват недоступен).
                        [tx, roomId, staffUserId, startsAt, endsAt,
                         cancelledCount, kCancelSql, kInsertBlockSql,
                         blockId, fire](const drogon::orm::Result&) {
                            *tx << kCancelSql << roomId << endsAt << startsAt
                                >>
                                [tx, roomId, staffUserId, startsAt, endsAt,
                                 cancelledCount, kInsertBlockSql, blockId,
                                 fire](const drogon::orm::Result& r) {
                                    *cancelledCount =
                                        static_cast<int64_t>(
                                            r.affectedRows());
                                    *tx << kInsertBlockSql << roomId
                                        << staffUserId << startsAt << endsAt
                                        >>
                                        [tx, blockId,
                                         fire](const drogon::orm::Result&) {
                                            *tx << "SELECT "
                                                   "last_insert_rowid() "
                                                   "AS id" >>
                                                [blockId](
                                                    const drogon::orm::Result&
                                                        rows) {
                                                    if (!rows.empty())
                                                        *blockId =
                                                            rows[0]["id"]
                                                                .as<int64_t>();
                                                    // Тело ответа собирает
                                                    // commit-callback.
                                                } >>
                                                [fire](const DrogonDbException&
                                                           e) {
                                                    LOG_ERROR
                                                        << "RoomsService::"
                                                           "block(rowid): "
                                                        << e.base().what();
                                                    fire(BlockStatus::Error,
                                                         Json::Value());
                                                };
                                        } >>
                                        [fire](const DrogonDbException& e) {
                                            LOG_ERROR << "RoomsService::block"
                                                         "(insert): "
                                                      << e.base().what();
                                            fire(BlockStatus::Error,
                                                 Json::Value());
                                        };
                                } >>
                                [fire](const DrogonDbException& e) {
                                    LOG_ERROR << "RoomsService::block"
                                                 "(cancel): "
                                              << e.base().what();
                                    fire(BlockStatus::Error, Json::Value());
                                };
                        } >>
                        [fire](const DrogonDbException& e) {
                            LOG_ERROR << "RoomsService::block(notify): "
                                      << e.base().what();
                            fire(BlockStatus::Error, Json::Value());
                        };

                    // Commit-callback: success → Created с телом ПОСЛЕ
                    // коммита; commit-fail → Error (после rollback callback
                    // не стреляет вовсе — v1.9.14; once-ограничитель
                    // страхует оба пути).
                    tx->setCommitCallback(
                        [roomId, roomName, startsAt, endsAt, reason,
                         cancelledCount, blockId,
                         fire](bool committed) {
                            if (!committed)
                            {
                                fire(BlockStatus::Error, Json::Value());
                                return;
                            }
                            Json::Value body(Json::objectValue);
                            body["id"] = *blockId;
                            body["room_id"] = roomId;
                            body["room"] = roomName;
                            body["starts_at"] = startsAt;
                            body["ends_at"] = endsAt;
                            body["status"] = "blocked";
                            body["reason"] = reason;
                            body["cancelled"] = *cancelledCount;
                            fire(BlockStatus::Created, std::move(body));
                        });
                });
        });
}

}  // namespace para
