#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

class ScheduleCache;

// Расписание группы (Э3 шаг 1; 02 §5.2: GET /v1/schedule/today,
// /v1/schedule/week?offset=, /v1/schedule/changes?days=; таблицы
// schedule_events/groups/profiles — миграции 0002/0001). Весь SQL —
// параметризованный и живёт здесь, не в контроллерах (AGENTS.md §5.5,
// 02 §6.4). Выборки по индексам 02 §4.2:
//   today/week — idx_schedule_group_time (group_id, starts_at): equality по
//     группе + диапазон дат + ORDER BY starts_at — один индекс на запрос;
//   changes — idx_schedule_group_status (group_id, status, starts_at):
//     equality по группе + IN по статусу + диапазон дат.
// Метки времени — строки ISO 8601 UTC проекта («…Z», как в фикстурах и
// SessionService::utcNow): сравнение/сортировка — лексикографические.
class ScheduleService
{
  public:
    // --- Даты (UTC проекта, YYYY-MM-DD) ----------------------------------
    // Сегодняшняя дата в UTC. Граница суток — по меткам «…Z» фикстур
    // (демо-неделя 2–8.11.2026 хранится в UTC).
    static std::string todayUtc();
    // Понедельник недели, содержащей date, со сдвигом offset недель (±).
    static std::string weekStart(const std::string& date, int offset);
    // date ± days суток (перекрёст календарных месяцев корректен).
    static std::string addDays(const std::string& date, int days);

    // --- SQL-выборки ------------------------------------------------------
    // Группа пользователя (profiles.group_id). NotFound — профиля нет
    // (staff, dean.demo — Р-6): группа не определена → пустой ответ 200.
    // Error — ошибка БД → fail-closed 500 (не «пустое расписание»).
    enum class GroupLookup
    {
        Found,
        NotFound,
        Error
    };
    static void groupOfUser(
        drogon::orm::DbClientPtr db,
        int64_t userId,
        std::function<void(GroupLookup, std::optional<int64_t> groupId)> cb);

    // События группы за день date включительно (по времени начала).
    static void eventsOnDay(
        drogon::orm::DbClientPtr db,
        int64_t groupId,
        const std::string& date,
        std::function<void(bool ok, Json::Value events)> cb);

    // События группы за полуинтервал [fromIso, toIsoExclusive) — «…Z»-строки.
    static void eventsOnRange(
        drogon::orm::DbClientPtr db,
        int64_t groupId,
        const std::string& fromIso,
        const std::string& toIsoExclusive,
        std::function<void(bool ok, Json::Value events)> cb);

    // Изменения (С3 «было → стало»): статусы moved/cancelled/substituted
    // (фиксированный белый список 02 §4.1 — SQL-литералы, не пользовательский
    // ввод), чья дата начала входит в последние days календарных суток,
    // заканчивающиеся today (включительно): [today-(days-1), today+1).
    static void recentChanges(
        drogon::orm::DbClientPtr db,
        int64_t groupId,
        int days,
        const std::string& today,
        std::function<void(bool ok, Json::Value events)> cb);

    // --- Ответы 02 §5.2 ---------------------------------------------------
    struct Result
    {
        bool ok = false;  // false — ошибка БД → контроллер отвечает 500
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/schedule/today → {"date": "YYYY-MM-DD", "events": [...]},
    // отсортированы по starts_at. staff (профиля нет) — 200 с пустым
    // events (группа не определена); today — UTC-дата «сейчас».
    static void today(drogon::orm::DbClientPtr db,
                      int64_t userId,
                      const std::string& today,
                      ResultCb cb);

    // GET /v1/schedule/week?offset=N → {"offset", "from", "to", "events"}:
    // 7 дней недели с понедельником weekStart(today, offset). offset=0 —
    // кэшируемая неделя (cache != nullptr): попадание — модель из кэша,
    // промах — выборка + put (TTL 60 с, хук инвалидации — Э6); offset≠0
    // кэш не использует (ключ — group_id, хранится только одна неделя).
    static void week(drogon::orm::DbClientPtr db,
                     int64_t userId,
                     int offset,
                     ScheduleCache* cache,
                     const std::string& today,
                     ResultCb cb);

    // GET /v1/schedule/changes?days=N → {"days", "changes": [...]}: каждый
    // элемент — событие + prev_* («было → стало»); days валидирует
    // контроллер (1..31). staff — 200 с пустым changes.
    static void changeList(drogon::orm::DbClientPtr db,
                           int64_t userId,
                           int days,
                           const std::string& today,
                           ResultCb cb);
};

}  // namespace para
