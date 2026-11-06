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
};

}  // namespace para
