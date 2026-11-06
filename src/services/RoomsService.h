#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Бронь аудитории (5.10; Э5а шаг 1; 02 §5.2: GET /v1/rooms — «каталог
// аудиторий: корпус, вместимость, оборудование», GET /v1/rooms/{id}/slots —
// «слоты дня: свободно / занято / ваша бронь»; таблицы rooms/room_bookings —
// миграция 0005). Весь SQL — параметризованный и живёт здесь, не в
// контроллерах (AGENTS.md §5.5, 02 §6.4).
//
// Сетка слотов — константа приложения (02 §4.3), не БД: слоты по 60 минут
// («спека: слоты по 60 минут» — 03 §4.9.5), окно учебного дня
// [kFirstSlotHour, kLastSlotHour) UTC = 08:00–22:00, 14 слотов. Часы —
// демо-конвенция шага 1 (в решениях не зафиксированы); FE рисует их как
// есть, сдвиг окна — правка этой константы без схемы.
//
// Состояние слота — по активным броням (status='booked'), пересекающим
// слот:
//   "free"   — свободно;
//   "booked" — занято (чужая активная бронь — формулировка 02 §5.2);
//   "mine"   — ваша бронь (строка принадлежит user_id сессии).
// Ровно три состояния 02 §5.2. Отменённая бронь (status='cancelled') слот
// НЕ занимает — статус снимает бронирование. Блокировки сотрудника
// (status='blocked') — механика ШАГА 3 Э5а (гайд BE §8): их отображение
// в сетке решается там же; шаг 1 осознанно не смешивает статусы — запрос
// по status='booked' в точности имплицирует условие частичного индекса
// idx_bookings_active ... WHERE status='booked' (02 §4.2 «активные брони
// по аудитории и времени»); статус IN ('booked','blocked') индекс НЕ
// использует (EXPLAIN: SCAN вместо SEARCH — проверено на этом шаге).
// Статус — фиксированный SQL-литерал из CHECK 0005, не пользовательский
// ввод.
//
// Выборка брони дня — room_id + диапазон starts_at: idx_bookings_active
// ((room_id, starts_at) WHERE status='booked') покрывает equality по
// room_id, диапазон по starts_at И порядок ORDER BY starts_at — один
// индекс на запрос (EXPLAIN QUERY PLAN проверен: SEARCH ... USING INDEX
// idx_bookings_active, без TEMP B-TREE). Пересечение слота и брони —
// row.starts_at < slotEnd AND row.ends_at > slotStart — считается в C++
// по строкам дня (точное «перекрытие», не «совпадение начала»: шаг 2
// бронирует ровно слоты, но контракт БД допускает произвольные интервалы).
//
// Даты/метки — строки ISO 8601 UTC проекта («…Z», как в фикстурах и
// ScheduleService): календарный день [dateT00:00:00Z, date+1T00:00:00Z).
class RoomsService
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД/невалидная дата → 500 контроллером
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // --- Даты (UTC проекта, YYYY-MM-DD) ----------------------------------
    // Сегодняшняя дата в UTC (дефолт ?date= — по 02 §5.2 «query: date»,
    // отсутствие — сегодня, как /v1/schedule/today).
    static std::string todayUtc();
    // Строгая валидация YYYY-MM-DD: формат, диапазоны и календарность
    // (2026-02-30 → false). Контроллер: невалидная/некорректная → 400
    // bad_request (02 §5.1). Сервис дополнительно fail-closed: невалидная
    // дата в slots() — ok=false (молча «весь день свободен» нельзя).
    static bool validDate(const std::string& date);

    // --- SQL-выборки ------------------------------------------------------
    // GET /v1/rooms → {"rooms": [...]} — каталог по id (ORDER BY id):
    // id/name/building/capacity/features. Пустая таблица — 200 с пустым
    // rooms (заполнение каталога — фикстуры/админ, не ошибка выборки).
    // query date/capacity 02 §5.2 — не в шаге 1 (постановка: «каталог
    // комнат»; фильтры бессмысленны без полей занятости шага 2 — там же).
    static void catalog(drogon::orm::DbClientPtr db, ResultCb cb);

    // Существование аудитории (404 контроллера). Found — строка есть
    // (body — id/name/building для шапки сетки), NotFound — аудитории нет,
    // Error — ошибка БД → fail-closed 500.
    enum class Lookup
    {
        Found,
        NotFound,
        Error
    };
    static void roomById(
        drogon::orm::DbClientPtr db,
        int64_t roomId,
        std::function<void(Lookup, Json::Value room)> cb);

    // GET /v1/rooms/{id}/slots?date= → {"room_id", "room", "building",
    // "date", "slots": [{"starts_at", "ends_at", "state"}, ...]} — сетка
    // дня по 60 минут, каждый слот со своим состоянием (см. выше);
    // starts_at/ends_at — «…Z»-строки сетки (UTC), state — free|booked|mine.
    // userId — из сессии (контроллер): «ваша бронь» определяется им;
    // владение БРОНЬЮ (не комнатой) — другой объект шага 2/3, здесь только
    // признак «эта строка моя». Исходы — раздельно (404/500 контроллером):
    // Ok — body сетки; NotFound — аудитории нет (после Own пути к данным
    // нет — удаление в гонке); Error — невалидная дата (контроллер до
    // вызова отвечает 400, здесь fail-closed) или ошибка БД → 500.
    enum class SlotsStatus
    {
        Ok,
        NotFound,
        Error
    };
    struct SlotsResult
    {
        SlotsStatus status = SlotsStatus::Error;
        Json::Value body;  // сетка дня при Ok
    };
    static void slots(drogon::orm::DbClientPtr db,
                      int64_t roomId,
                      int64_t userId,
                      const std::string& date,
                      std::function<void(SlotsResult)> cb);

    // --- Брони (Э5а шаг 2; 02 §5.2) ----------------------------------------
    // POST /v1/rooms/{id}/bookings — забронировать интервал. body: starts_at,
    // ends_at (02 §5.2) — строки ISO 8601 UTC «…Z» (формат фикстур/сеток);
    // слот 60 минут — частный случай (FE шлёт границы слота сетки), API
    // принимает произвольный интервал ends_at > starts_at: пересекающий
    // запрос 10:30–11:30 должен дойти до проверки пересечения (409), а не
    // отсеиваться «не слот» на валидации.
    //
    // Двойная защита от двойной брони (гайд BE §8):
    //   1) серверная проверка пересечений активных броней этой комнаты
    //      (SELECT status='booked' — idx_bookings_active, см. шапку slots):
    //      среди пересекающихся броней та, чьё starts_at совпадает с
    //      предлагаемым, — «тот же слот» → SlotTaken (400 «двойная бронь»);
    //      частичное перекрытие (10:30–11:30 против 10:00–11:00) → Conflict
    //      (409 «конфликт, только что заняли» — случай FE-сетки, 03 §4.9.5);
    //   2) частичный UNIQUE idx_bookings_active (room_id, starts_at)
    //      WHERE status='booked' — гонка двух параллельных POST (серверная
    //      проверка обеих прошла до INSERT'ов): исключение БД ловится и
    //      маппится в SlotTaken (400), НЕ в 500 (постановка шага 2).
    // Границы-касания не пересекаются: [10:00,11:00) и [11:00,12:00) —
    // полуинтервалы, вторая бронь начинается в момент окончания первой.
    enum class CreateStatus
    {
        Created,    // 201 + тело брони
        SlotTaken,  // 400 — двойная бронь того же слота (сервер или UNIQUE)
        Conflict,   // 409 — пересечение интервала («только что заняли»)
        InvalidBody,  // 400 — не ISO «…Z» / ends_at <= starts_at
        NotFound,   // 404 — аудитории нет
        Error       // 500 — ошибка БД (fail-closed)
    };
    struct CreateResult
    {
        CreateStatus status = CreateStatus::Error;
        Json::Value body;  // созданная бронь при Created
    };
    static void create(drogon::orm::DbClientPtr db,
                       int64_t roomId,
                       int64_t userId,
                       const std::string& startsAt,
                       const std::string& endsAt,
                       std::function<void(CreateResult)> cb);

    // GET /v1/room-bookings — МОИ активные брони (status='booked'), по
    // starts_at (idx_bookings_user (user_id, starts_at) — 02 §4.2 «мои
    // брони»); JOIN rooms — имя/корпус для строки списка «Мои брони».
    // upcoming=true (?upcoming=1, 02 §5.2) — только предстоящие
    // (ends_at >= nowIso); nowIso передаёт контроллер (SessionService::
    // utcNow) — wall-clock в тестах не участвует. Отменённые не возвращаются.
    static void myBookings(drogon::orm::DbClientPtr db,
                           int64_t userId,
                           bool upcoming,
                           const std::string& nowIso,
                           ResultCb cb);

    // DELETE /v1/room-bookings/{id} — отмена СВОЕЙ активной брони. Владение
    // проверяет КОНТРОЛЛЕР через OwnershipGuard::check(Table::RoomBookings)
    // ДО вызова (правило код-ревью OwnershipGuard.h; шаг 8 автотеста
    // 02 §9.3: чужая бронь по подставленному адресу → 403 — 01 §5.10).
    // Выборка/UPDATE здесь — по PK БЕЗ user_id-условия: владение проверено
    // ровно один раз, guard'ом (постановка Э4 шаг 2). Отмена — статус
    // 'cancelled' (CHECK 0005), НЕ удаление строки: история/индексы целы.
    enum class CancelStatus
    {
        Canceled,   // 204 — условный UPDATE перевёл booked → cancelled
        NotActive,  // 409 — бронь не в booked (уже отменена/блокировка)
        NotFound,   // 404 — строки нет (после Own — гонка; не маскируется)
        Error       // 500 — ошибка БД (fail-closed)
    };
    static void cancelById(drogon::orm::DbClientPtr db,
                           int64_t bookingId,
                           std::function<void(CancelStatus)> cb);
};

}  // namespace para
