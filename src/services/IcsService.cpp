#include "services/IcsService.h"

#include <cstdio>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

#include <drogon/drogon.h>

#include "services/IcsBuilder.h"
#include "services/ScheduleService.h"
#include "services/SessionService.h"

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// Полный диапазон дат для выборки расписания группы: весь горизонт таблицы
// (календарь — за все недели, не только демо-неделю). Полуинтервал по
// idx_schedule_group_time, как в ScheduleService::eventsOnRange.
constexpr const char* kAllFrom = "1970-01-01T00:00:00Z";
constexpr const char* kAllTo = "3000-01-01T00:00:00Z";

// Длительность персональной даты сессии: в session_plan есть только начало
// (миграция 0002); VEVENT обязан иметь конец — прототип ставит 2 часа
// (консультация/экзамен; допущение фиксируется журналом шага).
constexpr int kPlanEventHours = 2;

std::string addHoursIso(const std::string& iso, int hours)
{
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%dZ", &y, &mo, &d, &h, &mi, &s) !=
        6)
        return iso;  // не парсится — DTEND останется равным DTSTART
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = s;
    tm.tm_isdst = 0;
    const std::time_t t = timegm(&tm) + static_cast<std::time_t>(hours) * 3600;
    std::tm out{};
    gmtime_r(&t, &out);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &out);
    return buf;
}

IcsBuilder::CalendarItem eventItem(const Json::Value& e)
{
    // JSON-событие (ScheduleService::eventsOnRange) → запись календаря;
    // prev_room — null или строка (Json::Value).
    std::optional<std::string> prevRoom;
    if (e.isMember("prev_room") && !e["prev_room"].isNull())
        prevRoom = e["prev_room"].asString();
    return IcsBuilder::CalendarItem{
        "para-event-" + std::to_string(e["id"].as<int64_t>()) + "@para.demo",
        e["starts_at"].asString(),
        e["ends_at"].asString(),
        IcsBuilder::summaryFor(e["discipline"].asString(),
                               e["room"].asString(),
                               e["status"].asString(),
                               prevRoom),
        e["room"].asString()};
}

// Персональные даты session_plan владельца (idx_session_plan_user — 02 §4.2,
// «Экран "Сессия"»): id, kind, starts_at, room + название дисциплины.
void planOf(drogon::orm::DbClientPtr db,
            int64_t userId,
            std::function<void(bool ok, std::vector<IcsBuilder::CalendarItem>)> cb)
{
    const std::string sql =
        "SELECT p.id, p.kind, p.starts_at, p.room, d.title AS discipline_title "
        "FROM session_plan p JOIN disciplines d ON d.id = p.discipline_id "
        "WHERE p.user_id = $1 ORDER BY p.starts_at";
    *db << sql << userId >>
        [cb](const drogon::orm::Result& rows) {
            std::vector<IcsBuilder::CalendarItem> items;
            for (const auto& row : rows)
            {
                const std::string start = row["starts_at"].as<std::string>();
                items.push_back(IcsBuilder::CalendarItem{
                    "para-plan-" + std::to_string(row["id"].as<int64_t>()) +
                        "@para.demo",
                    start,
                    addHoursIso(start, kPlanEventHours),
                    IcsBuilder::summaryForPlan(row["kind"].as<std::string>(),
                                               row["discipline_title"]
                                                   .as<std::string>()),
                    row["room"].as<std::string>()});
            }
            cb(true, std::move(items));
        } >>
        // Копия cb в обе ветки цепочки — move в первой оставил бы второй
        // пустой std::function (std::bad_function_call, урок Э2 шаг 1).
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "IcsService::planOf: " << e.base().what();
            cb(false, {});
        };
}

}  // namespace

void IcsService::createToken(
    drogon::orm::DbClientPtr db,
    int64_t userId,
    std::function<void(bool ok, const std::string& token)> cb)
{
    // 256 бит энтропии → 64 hex-символа (Р-11, правки по гейту Э3 раунд 1:
    // проверка «токенДлина» — приватный токен живёт в URL как bearer-ключ
    // GET /ics; длина — единственная усиливаемая защита, 128 бит ниже
    // порога гейта).
    const std::string token = SessionService::generateToken(32);
    const std::string now = SessionService::utcNow();
    // user_id — PRIMARY KEY ics_tokens: UPSERT и есть ротация (строка одна,
    // прежний токен перестаёт находиться — он заменён этим).
    *db << "INSERT INTO ics_tokens (user_id, token, created_at, revoked) "
           "VALUES ($1, $2, $3, 0) "
           "ON CONFLICT(user_id) DO UPDATE SET token = excluded.token, "
           "created_at = excluded.created_at, revoked = 0"
        << userId << token << now >>
        [cb, token](const drogon::orm::Result&) { cb(true, token); } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "IcsService::createToken(" << userId
                      << "): " << e.base().what();
            cb(false, std::string());  // 500 без деталей (02 §5.1)
        };
}

void IcsService::revokeToken(drogon::orm::DbClientPtr db,
                             int64_t userId,
                             std::function<void(bool ok)> cb)
{
    *db << "UPDATE ics_tokens SET revoked = 1 WHERE user_id = $1 AND "
           "revoked = 0"
        << userId >>
        [cb](const drogon::orm::Result&) { cb(true); } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "IcsService::revokeToken(" << userId
                      << "): " << e.base().what();
            cb(false);
        };
}

void IcsService::activeToken(drogon::orm::DbClientPtr db,
                             int64_t userId,
                             std::function<void(ActiveToken)> cb)
{
    // 8.ii: тот же источник, что у фида GET /ics/{token} (строка с
    // revoked = 0) — один активный токен на пользователя по схеме
    // (user_id PRIMARY KEY); пустая выборка — «нет подписки» → 404.
    *db << "SELECT token, created_at FROM ics_tokens "
           "WHERE user_id = $1 AND revoked = 0"
        << userId >>
        [cb](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                cb(ActiveToken{true, false, std::string(), std::string()});
                return;
            }
            ActiveToken t;
            t.ok = true;
            t.found = true;
            t.token = rows[0]["token"].as<std::string>();
            t.createdAt = rows[0]["created_at"].as<std::string>();
            cb(std::move(t));
        } >>
        // cb копируется в обе ветки цепочки (урок Э2 шаг 1).
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "IcsService::activeToken: " << e.base().what();
            cb(ActiveToken{});
        };
}

std::string IcsService::stampNow()
{
    std::tm tm{};
    const std::time_t now = std::time(nullptr);
    gmtime_r(&now, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y%m%dT%H%M%SZ", &tm);
    return buf;
}

void IcsService::calendarByToken(drogon::orm::DbClientPtr db,
                                 const std::string& token,
                                 std::function<void(Calendar)> cb)
{
    // Аутентификация — сам токен (02 §6.1: /ics/{token} без cookie):
    // строка с token и revoked = 0.
    *db << "SELECT user_id FROM ics_tokens WHERE token = $1 AND revoked = 0"
        << token >>
        [db, cb](const drogon::orm::Result& rows) mutable {
            if (rows.empty())
            {
                // Неверный или отозванный токен → 404 (02 §5.2), не 401:
                // токен и есть аутентификация, «нет токена» — нет объекта.
                cb(Calendar{true, false, std::string()});
                return;
            }
            const int64_t userId = rows[0]["user_id"].as<int64_t>();
            // Staff без профиля (NotFound) — календарь из session_plan
            // владельца; группа не определена. Все внутренние лямбды
            // захватывают КОПИИ (не ссылки на члены внешних замыканий):
            // каждая переживает внешний колбэк, который уничтожается сразу
            // после передачи дальше по цепочке.
            ScheduleService::groupOfUser(
                db,
                userId,
                [db, userId, cb](ScheduleService::GroupLookup lookup,
                                 std::optional<int64_t> groupId) mutable {
                    auto fail = [cb]() mutable {
                        cb(Calendar{false, true, std::string()});
                    };
                    auto finish =
                        [cb](std::vector<IcsBuilder::CalendarItem> items) mutable {
                            Calendar c;
                            c.ok = true;
                            c.found = true;
                            c.ics =
                                IcsBuilder::build(items, IcsService::stampNow());
                            cb(std::move(c));
                        };
                    auto planThenFinish =
                        [fail, finish](bool ok,
                                       std::vector<IcsBuilder::CalendarItem>
                                           plan) mutable {
                            if (!ok)
                            {
                                fail();
                                return;
                            }
                            finish(std::move(plan));
                        };
                    if (lookup == ScheduleService::GroupLookup::NotFound)
                    {
                        planOf(db, userId, std::move(planThenFinish));
                        return;
                    }
                    if (lookup == ScheduleService::GroupLookup::Error)
                    {
                        fail();
                        return;
                    }
                    // Расписание группы: все статусы (moved/cancelled
                    // остаются в календаре с пометкой SUMMARY).
                    ScheduleService::eventsOnRange(
                        db,
                        *groupId,
                        kAllFrom,
                        kAllTo,
                        [db, userId, fail, finish](bool ok,
                                                   Json::Value events) mutable {
                            if (!ok)
                            {
                                fail();
                                return;
                            }
                            std::vector<IcsBuilder::CalendarItem> items;
                            for (const auto& e : events)
                                items.push_back(eventItem(e));
                            planOf(
                                db,
                                userId,
                                [items = std::move(items), fail, finish](
                                    bool ok,
                                    std::vector<IcsBuilder::CalendarItem> plan)
                                    mutable {
                                    if (!ok)
                                    {
                                        fail();
                                        return;
                                    }
                                    for (auto& item : plan)
                                        items.push_back(std::move(item));
                                    finish(std::move(items));
                                });
                        });
                });
        } >>
        // Колбэк копируется в обе ветки цепочки (result/exception) —
        // OwnershipGuard.cpp:56, урок Э2 шаг 1.
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "IcsService::calendarByToken: " << e.base().what();
            cb(Calendar{false, false, std::string()});
        };
}

}  // namespace para
