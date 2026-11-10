#include "services/DashboardService.h"

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "services/ApplicationsService.h"
#include "services/GradesService.h"
#include "services/NotificationsFeedService.h"
#include "services/ScheduleService.h"
#include "services/SessionService.h"
#include "services/UserService.h"

namespace para {

bool DashboardService::isActiveApplicationStatus(const std::string& status)
{
    return status == "submitted" || status == "accepted" ||
           status == "in_progress" || status == "need_info" ||
           status == "ready";
}

namespace {

// Первое ЛИЧНОЕ имя из fio. Формат фикстур — «Фамилия Имя Отчество»
// (согласован с маской «А. Соколова» 02 §5.2), вайрфрейм 03 §4.2
// приветствует по имени («Привет, Аня») → имя = второе слово;
// однословный fio → само слово. Правки по ревью экрана (было — первое
// слово, т.е. фамилия).
std::string personalName(const std::string& fio)
{
    const auto firstSpace = fio.find(' ');
    if (firstSpace == std::string::npos)
        return fio;  // однословный fio
    const auto secondSpace = fio.find(' ', firstSpace + 1);
    if (secondSpace == std::string::npos)
        return fio.substr(firstSpace + 1);  // «Фамилия Имя»
    return fio.substr(firstSpace + 1, secondSpace - firstSpace - 1);
}

// Общий стейт агрегата. Шесть асинхронных вызовов сервисов пишут СЕМЬ
// полей (schedule отдаёт today и next_class одним ответом): каждый
// завершившийся источник уменьшает pending под замком; последний
// достроивший собирает ответ. Мьютекс — агрегат не опирается на топологию
// потоков DbClient (config: connection_number=1, но это свойство конфига,
// не контракта).
struct Aggregate
{
    std::mutex mutex;
    int pending = 7;
    Json::Value body;
    std::function<void(const DashboardService::Result&)> cb;
};
using AggregatePtr = std::shared_ptr<Aggregate>;

// Поле виджета готово: записать значение (пустой key — только декремент),
// при ошибке источника пометить ключ в degraded (fail-soft — см. шапку
// класса). Последний достроивший отдаёт Result 200. Единственная точка
// ответа: cb стреляет ровно один раз при pending == 0.
void widgetDone(const AggregatePtr& state, std::string key, Json::Value value,
                bool failed)
{
    std::unique_lock lock(state->mutex);
    if (!key.empty())
        state->body[std::move(key)] = std::move(value);
    if (failed && !key.empty())
        state->body["degraded"].append(key);
    if (--state->pending > 0)
        return;
    auto cb = std::move(state->cb);
    Json::Value body = std::move(state->body);
    lock.unlock();
    cb(DashboardService::Result{true, std::move(body)});
}

}  // namespace

void DashboardService::dashboard(drogon::orm::DbClientPtr db, int64_t userId,
                                 ResultCb cb)
{
    auto state = std::make_shared<Aggregate>();
    state->cb = std::move(cb);
    // Деградированные значения по умолчанию: ошибка источника или незавершённый
    // источник не ломают контракт — поле есть; degraded накапливается ниже.
    state->body["profile"] = Json::Value(Json::objectValue);
    state->body["profile"]["short_name"] = std::string();
    state->body["next_class"] = Json::Value();  // jsoncpp null
    state->body["today"] = Json::Value(Json::arrayValue);
    state->body["deadlines"] = Json::Value(Json::arrayValue);
    state->body["applications"] = Json::Value(Json::arrayValue);
    state->body["news"] = Json::Value(Json::arrayValue);
    state->body["unread_count"] = 0;
    state->body["degraded"] = Json::Value(Json::arrayValue);

    // --- 1/7 профиль: short_name — личное имя из fio -----------------------
    UserService::loadProfile(
        db, userId, [state](std::optional<Json::Value> profile) {
            // nullopt — staff (профиля нет, Р-6) или ошибка БД: имя
            // остаётся пустым; в degraded профиль не попадает (сервис не
            // различает эти случаи — ложно помечали бы staff, см. шапку).
            if (profile.has_value())
            {
                const Json::Value& fio = (*profile)["fio"];
                Json::Value p(Json::objectValue);
                p["short_name"] =
                    fio.isString() ? personalName(fio.asString())
                                   : std::string();
                widgetDone(state, "profile", std::move(p), false);
                return;
            }
            widgetDone(state, "profile", Json::Value(), false);
        });

    // --- 2–3/7 расписание: «Сегодня» + «Следующая пара» --------------------
    ScheduleService::today(
        db, userId, ScheduleService::todayUtc(),
        [state](ScheduleService::Result r) {
            Json::Value today(Json::arrayValue);
            Json::Value next;  // null — «пары кончились/нет»
            if (r.ok)
            {
                today = std::move(r.body["events"]);
                // Ближайшая незаконченная пара: ends_at > «сейчас» —
                // лексикографическое сравнение ISO-«…Z»-строк проекта
                // (ScheduleService.h). Начавшаяся пара — тоже «следующая»:
                // FE показывает оверлайн «ИДЁТ СЕЙЧАС» (03 §4.2).
                const std::string now = SessionService::utcNow();
                for (const auto& e : today)
                {
                    if (e["ends_at"].asString() > now)
                    {
                        next = e;
                        break;
                    }
                }
            }
            widgetDone(state, "today", std::move(today), !r.ok);
            widgetDone(state, "next_class", std::move(next), !r.ok);
        });

    // --- 4/7 дедлайны: ближайшие события сессии (первые 3 по starts_at) ---
    GradesService::exams(
        db, userId, [state](GradesService::Result r) {
            Json::Value deadlines(Json::arrayValue);
            if (r.ok)
            {
                const Json::Value& exams = r.body["exams"];
                for (const auto& e : exams)
                {
                    if (deadlines.size() >= 3)
                        break;
                    deadlines.append(e);
                }
            }
            widgetDone(state, "deadlines", std::move(deadlines), !r.ok);
        });

    // --- 5/7 заявки: последние активные (список — новые сверху) -----------
    ApplicationsService::list(
        db, userId, std::nullopt,
        [state](ApplicationsService::Result r) {
            Json::Value applications(Json::arrayValue);
            if (r.ok)
            {
                const Json::Value& list = r.body["applications"];
                for (const auto& a : list)
                {
                    if (!isActiveApplicationStatus(a["status"].asString()))
                        continue;
                    if (applications.size() >= 3)
                        break;
                    applications.append(a);
                }
            }
            widgetDone(state, "applications", std::move(applications),
                       !r.ok);
        });

    // --- 6/7 «Что нового»: последние уведомления (первые 3 ленты) ---------
    NotificationsFeedService::Query full;
    NotificationsFeedService::feed(
        db, userId, full,
        [state](NotificationsFeedService::FeedResult r) {
            Json::Value news(Json::arrayValue);
            if (r.status == NotificationsFeedService::FeedStatus::Ok)
            {
                const Json::Value& notifications = r.body["notifications"];
                for (const auto& n : notifications)
                {
                    if (news.size() >= 3)
                        break;
                    news.append(n);
                }
            }
            widgetDone(state, "news", std::move(news),
                       r.status != NotificationsFeedService::FeedStatus::Ok);
        });

    // --- 7/7 непрочитанные: длина первой страницы unread=1 (0..20) --------
    NotificationsFeedService::Query unread;
    unread.unreadOnly = true;
    NotificationsFeedService::feed(
        db, userId, unread,
        [state](NotificationsFeedService::FeedResult r) {
            int count = 0;
            const bool failed =
                r.status != NotificationsFeedService::FeedStatus::Ok;
            if (!failed && r.body["notifications"].isArray())
                count = static_cast<int>(r.body["notifications"].size());
            widgetDone(state, "unread_count", Json::Value(count), failed);
        });
}

}  // namespace para
