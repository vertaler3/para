#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/SessionService.h"

namespace para {

// Staff-внесение изменений в расписание (Э6 шаг 4; гайд BE §9; 02 §5.2:
// POST /v1/staff/schedule/changes — 201 · 403 (не staff, StaffFilter) · 400).
// «было → стало» — prev_*-поля schedule_events (миграция 0002): prev_starts_at
// /prev_room (перенос), prev_teacher_id (замена). Изменение — в транзакции
// (условный UPDATE по текущему статусу — защита от гонки); ПОСЛЕ коммита —
// инвалидация кэша недели группы (хук ScheduleCache::invalidate из Э3,
// передаётся функцией — контроллер биндит кэш, юнит-тест счётчик) и
// АВТОРАССЫЛКА: уведомление type='schedule' КАЖДОМУ студенту группы через
// NotifyDispatcher (шаг 1: тумблеры/тихие часы/срочность «ближайшей пары»
// применяются к каждому получателю; объявление об изменении пары, которая
// начинается в течение 24 ч, тихие часы не задерживают — 01 §5.6).
class ScheduleChangeService
{
  public:
    enum class Action
    {
        Move,        // перенос: новые starts_at и/или room
        Cancel,      // отмена
        Substitute,  // замена преподавателя (teacher_id — замещающий)
    };

    static std::optional<Action> actionFromString(const std::string& s);
    static const char* actionName(Action action);

    // Валидные значения полей тела (02 §5.2): action — словарь выше;
    // move требует starts_at и/или room; substitute требует teacher_id;
    // reason — опционален, попадает в тело уведомления.
    struct Input
    {
        int64_t eventId = 0;
        Action action = Action::Move;
        std::optional<std::string> startsAt;  // ISO «YYYY-MM-DDTHH:MM:SSZ»
        std::optional<std::string> room;
        std::optional<int64_t> teacherId;
        std::string reason;
    };

    // Строгая календарность метки ISO «…Z» (формат DDL/фикстур; timegm
    // нормализует некалендарные даты — обратная сборка строки ловит это,
    // приём validDate RoomsService).
    static bool validIsoUtc(const std::string& iso);

    enum class Outcome
    {
        Ok,          // 201 — изменено, инвалидация и рассылка запущены
        NotFound,    // 404 — события нет
        BadRequest,  // 400 — недопустимое действие/поля/повторная отмена
        Error,       // 500 — ошибка БД (fail-closed, ничего не изменено)
    };
    struct Result
    {
        Outcome outcome = Outcome::Error;
        Json::Value body;  // 201: {action, event{…02 §5.2…}, recipients}
    };
    using ResultCb = std::function<void(Result)>;

    // invalidate(groupId) — хук инвалидации кэша недели (после коммита).
    using InvalidateFn = std::function<void(int64_t groupId)>;

    static void applyChange(drogon::orm::DbClientPtr db,
                            const SessionInfo& staff,
                            const Input& in,
                            InvalidateFn invalidate,
                            ResultCb cb);
};

}  // namespace para
