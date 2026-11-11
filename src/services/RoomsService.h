#pragma once

#include <functional>
#include <optional>
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
// Состояние слота — по строкам room_bookings, пересекающим слот и
// занимающим его (шаг 3 Э5а):
//   "free"   — свободно;
//   "booked" — занято (чужая активная бронь ИЛИ блокировка деканата:
//              заблокированный слот не «ваша бронь» и не тапается —
//              честная сетка для FE; статус-литералы CHECK 0005);
//   "mine"   — ваша бронь (активная бронь, user_id = сессии).
// Ровно три состояния 02 §5.2. Отменённая бронь (status='cancelled') слот
// НЕ занимает. Выборка — UNION ALL двух статус-веток: 'booked' ветка в
// точности имплицирует условие частичного индекса idx_bookings_active
// (02 §4.2; EXPLAIN: SEARCH), 'blocked' ветка читает единичные строки
// деканата (SCAN); общий статус IN ('booked','blocked') индекс НЕ
// использует целиком (проверено EXPLAIN на шаге 1) — потому две ветки.
//
// Пересечение слота и строки — row.starts_at < slotEnd AND row.ends_at >
// slotStart — точное перекрытие по строкам дня, не «совпадение начала».
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
    // query контракта 02 §5.2 (правки по гейту Э5а, раунд 1 — до этого
    // строка каталога была единственной с нереализованным query):
    //   minCapacity (query capacity) — фильтр «вместимость не меньше»
    //     (найти аудиторию на N человек), capacity >= $1;
    //   query date — принимается и валидируется контроллером (YYYY-MM-DD;
    //     невалидная → 400, единый стиль ?date= сетки), на выдачу каталога
    //     НЕ влияет: поля занятости дня в контракте каталога 02 §5.2 не
    //     описаны (сетка занятости — /v1/rooms/{id}/slots?date=).
    static void catalog(drogon::orm::DbClientPtr db,
                        const std::optional<int64_t>& minCapacity,
                        ResultCb cb);

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
        Blocked,    // 409 — интервал пересекает блокировку деканата (шаг 3)
        InvalidBody,  // 400 — не ISO «…Z» / ends_at <= starts_at
        NotFound,   // 404 — аудитории нет
        Error       // 500 — ошибка БД (fail-closed)
    };
    struct CreateResult
    {
        CreateStatus status = CreateStatus::Error;
        Json::Value body;  // созданная бронь при Created
    };
    // Проверка пересечений (шаг 3): активные брони ('booked' —
    // idx_bookings_active: тот же слот → SlotTaken, перекрытие → Conflict)
    // И блокировки деканата ('blocked': перекрытие/тот же слот → Blocked —
    // бронирование поверх блокировки исключено; UNIQUE частичный индекс
    // blocked-строки не защищает — только серверная проверка).
    // С шага 7 Э8 проверка и вставка — ОДИН атомарный guarded
    // INSERT..SELECT (гонка Э5а: прежние раздельные SELECT+INSERT пропускали
    // параллельные POST с разными пересекающимися интервалами); если
    // предикат NOT EXISTS не вставил строку (affectedRows = 0), статус
    // занятости классифицирует classifyTaken.
    static void create(drogon::orm::DbClientPtr db,
                       int64_t roomId,
                       int64_t userId,
                       const std::string& startsAt,
                       const std::string& endsAt,
                       std::function<void(CreateResult)> cb);

    // Внутренняя (вызывается только из create): классификация занятого
    // слота после нулевого affectedRows guarded INSERT — прежние две
    // SELECT-проверки: «тот же starts_at» → SlotTaken (400), частичное
    // перекрытие 'booked' → Conflict (409), перекрытие 'blocked' →
    // Blocked; ничего не нашлось (гонка с отменой между INSERT и
    // классификацией) — консервативный Conflict.
    static void classifyTaken(drogon::orm::DbClientPtr db,
                              int64_t roomId,
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

    // POST /v1/staff/rooms/{id}/block — блокировка интервала деканатом
    // (Э5а шаг 3; 02 §5.2: body starts_at, ends_at, reason; 01 §5.10:
    // «активная бронь снимается, автору уходит уведомление»). Атомарно в
    // одной транзакции (как AppWorkflow::apply):
    //   1) активные брони комнаты, пересекающие интервал
    //      ('booked' — idx_bookings_active), получают status='cancelled'
    //      (строки сохраняются — история; 'blocked' статус для СНЯТОЙ брони
    //      не используется: блокировка — ОТДЕЛЬНАЯ строка этого статуса);
    //   2) автору каждой снятой брони — уведомление
    //      (notifications: type 'schedule' — словарь CHECK 0004
    //      ('schedule','deadline','application','announcement'); бронь 5.10
    //      живёт в шапке расписания 03 §4.9.5; deep_link '/room-bookings');
    //   3) INSERT строки блокировки status='blocked' (user_id = staff).
    // Уведомление внутри транзакции: ошибка откатывает блокировку вместе
    // со снятиями (паттерн staff-статуса Э5 шаг 3).
    // Блокировки НЕ мешают друг другу (UNIQUE частичный — только 'booked'),
    // повторная блокировка того же слота — новая строка, 201.
    enum class BlockStatus
    {
        Created,      // 201 + тело блокировки
        InvalidBody,  // 400 — не ISO «…Z» / ends_at <= starts_at
        NotFound,     // 404 — аудитории нет
        Error         // 500 — ошибка БД (fail-closed: транзакция откатится)
    };
    struct BlockResult
    {
        BlockStatus status = BlockStatus::Error;
        Json::Value body;  // {id, room_id, room, starts_at, ends_at,
                           //  status:"blocked", reason, cancelled}
    };
    static void block(drogon::orm::DbClientPtr db,
                      int64_t roomId,
                      int64_t staffUserId,
                      const std::string& startsAt,
                      const std::string& endsAt,
                      const std::string& reason,
                      std::function<void(BlockResult)> cb);
};

}  // namespace para
