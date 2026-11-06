#include "services/NotifyDispatcher.h"

#include <cstdio>
#include <ctime>
#include <map>
#include <mutex>
#include <utility>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Строка notifications (02 §4.1, миграция 0004): created_at — DEFAULT DDL
// (strftime UTC), read_at — NULL (непрочитанное).
const std::string kInsertSql =
    "INSERT INTO notifications (user_id, type, title, body, deep_link) "
    "VALUES ($1, $2, $3, $4, $5)";

// 24 часа в секундах — окно срочности «ближайшей пары» (01 §5.6).
constexpr std::time_t kUrgentWindowSec = 24 * 3600;

// --- статики диспетчера (push-колбэк, часы, очередь) -----------------------
// IO-потоки drogon вызывают notify() конкурентно — всё под одним мьютексом;
// invoke-снапшоты (копия std::function вне лока) — чтобы колбэк не мог
// вывавать resetForTests/setPushFn изнутри (рекурсивный лок невозможен).
std::mutex gMutex;
NotifyDispatcher::PushFn gPushFn;   // точка интеграции шага 3 (WsCtrl)
NotifyDispatcher::ClockFn gClock;   // тестовый шов времени
std::vector<DeferredNotification> gPendingDigest;  // очередь шага 2 (дайджест)

NotifyDispatcher::PushFn pushFnSnapshot()
{
    std::lock_guard<std::mutex> lock(gMutex);
    return gPushFn;
}

// "HH:MM" из quiet_hours_json → минуты суток [0;1440); нечитаемое → -1.
int parseHm(const Json::Value& v)
{
    if (!v.isString())
        return -1;
    int h = 0, m = 0;
    if (sscanf(v.asCString(), "%d:%d", &h, &m) != 2 || h < 0 || h > 23 ||
        m < 0 || m > 59)
        return -1;
    return h * 60 + m;
}

// «YYYY-MM-DDTHH:MM:SSZ» → time_t UTC; нечитаемое/некалендарное → -1.
// Формат — как в DDL/фикстурах (strftime '%Y-%m-%dT%H:%M:%SZ', 02 §4.1).
// timegm нормализует некалендарные даты (2026-02-30 → 02.03) — обратная
// сборка строки ловит это (приём validDate, RoomsService).
std::time_t isoUtcToTime(const std::string& iso)
{
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    if (sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &s) != 6)
        return -1;
    if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 ||
        mi > 59 || s > 59)
        return -1;
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = s;
    tm.tm_isdst = 0;
    const std::time_t t = timegm(&tm);
    std::tm back{};
    gmtime_r(&t, &back);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &back);
    return (iso == buf) ? t : -1;  // нормализовалось → дата не календарная
}

std::string timeToIsoUtc(std::time_t t)
{
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

int minuteOfDayOf(const std::chrono::system_clock::time_point& tp)
{
    const std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
    gmtime_r(&t, &tm);  // UTC без DST — минуты суток линейны (02 §4.3)
    return tm.tm_hour * 60 + tm.tm_min;
}

}  // namespace

const char* notifyTypeName(NotifyType type)
{
    switch (type)
    {
        case NotifyType::Schedule:
            return "schedule";
        case NotifyType::Deadline:
            return "deadline";
        case NotifyType::Application:
            return "application";
        case NotifyType::Announcement:
            return "announcement";
    }
    return "schedule";  // недостижимо — все значения enum покрыты
}

std::optional<NotifyType> notifyTypeFromString(const std::string& s)
{
    if (s == "schedule")
        return NotifyType::Schedule;
    if (s == "deadline")
        return NotifyType::Deadline;
    if (s == "application")
        return NotifyType::Application;
    if (s == "announcement")
        return NotifyType::Announcement;
    return std::nullopt;  // вне словаря CHECK миграции 0004
}

void NotifyDispatcher::setPushFn(PushFn fn)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gPushFn = std::move(fn);
}

void NotifyDispatcher::setClock(ClockFn fn)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gClock = std::move(fn);
}

std::chrono::system_clock::time_point NotifyDispatcher::nowUtc()
{
    std::lock_guard<std::mutex> lock(gMutex);
    if (gClock)
        return gClock();
    return std::chrono::system_clock::now();
}

std::string NotifyDispatcher::toIsoUtc(std::chrono::system_clock::time_point tp)
{
    return timeToIsoUtc(std::chrono::system_clock::to_time_t(tp));
}

std::string NotifyDispatcher::nowIsoUtc()
{
    return toIsoUtc(nowUtc());
}

bool NotifyDispatcher::inQuietHours(const std::string& quietHoursJson,
                                    int minuteOfDayUtc)
{
    // Дефолт DDL: {"from":"23:00","to":"07:00"}; синонимы start/end — на
    // случай редактирования настройки руками (02 §5.6 называет {start,end}).
    Json::Value quiet;
    if (!Json::Reader().parse(quietHoursJson, quiet) || !quiet.isObject())
        return false;
    int from = parseHm(quiet["from"]);
    int to = parseHm(quiet["to"]);
    if (from < 0)
        from = parseHm(quiet["start"]);
    if (to < 0)
        to = parseHm(quiet["end"]);
    if (from < 0 || to < 0 || from == to)  // from == to — пустой интервал
        return false;
    if (from < to)  // внутри одних суток: [01:00, 05:00)
        return minuteOfDayUtc >= from && minuteOfDayUtc < to;
    // Через полночь: [23:00, 24:00) ∪ [00:00, 07:00)
    return minuteOfDayUtc >= from || minuteOfDayUtc < to;
}

bool NotifyDispatcher::isUrgentSchedule(
    const std::optional<std::string>& eventStartsAtUtc,
    const std::chrono::system_clock::time_point& now)
{
    if (!eventStartsAtUtc.has_value())
        return false;
    const std::time_t starts = isoUtcToTime(*eventStartsAtUtc);
    if (starts < 0)
        return false;
    const std::time_t t =
        std::chrono::system_clock::to_time_t(now);
    const std::time_t delta = starts - t;  // >0 — в будущем
    return delta >= 0 && delta <= kUrgentWindowSec;
}

NotifyOutcome NotifyDispatcher::decide(const SettingsRecord& settings,
                                       NotifyType type,
                                       bool urgent,
                                       int minuteOfDayUtc)
{
    // 1. Тумблер типа (02 §4.1: notify_schedule/notify_deadlines/
    //    notify_applications). У announcement тумблера нет — осознанно
    //    (01 §4.5: три тумблера покрывают весь push-канал).
    switch (type)
    {
        case NotifyType::Schedule:
            if (!settings.notifySchedule)
                return NotifyOutcome::SuppressedByToggle;
            break;
        case NotifyType::Deadline:
            if (!settings.notifyDeadlines)
                return NotifyOutcome::SuppressedByToggle;
            break;
        case NotifyType::Application:
            if (!settings.notifyApplications)
                return NotifyOutcome::SuppressedByToggle;
            break;
        case NotifyType::Announcement:
            // Объявления: всегда в ленту/дайджест, мимо тихих часов и push
            // (01 §5.6: «push-ем их не отправляем, поэтому в тихие часы
            // не попадают»).
            return NotifyOutcome::Sent;
    }

    // 2. Тихие часы (02 §5.6: правило на сервере при отправке): ночное
    //    не-срочное уведомление не создаётся сразу — в утренний дайджест.
    //    Срочный перенос ближайшей пары тихие часы не задерживают.
    if (!urgent && inQuietHours(settings.quietHoursJson, minuteOfDayUtc))
        return NotifyOutcome::DeferredToDigest;
    return NotifyOutcome::Sent;
}

void NotifyDispatcher::pushLive(const NotifyInput& in)
{
    if (in.type == NotifyType::Announcement)
        return;  // объявления push-ем не отправляются (01 §4.5)
    const auto fn = pushFnSnapshot();
    if (!fn)
        return;  // реестра WS ещё нет (шаг 3) — пуш не стреляет
    Json::Value payload(Json::objectValue);
    payload["type"] = notifyTypeName(in.type);
    payload["title"] = in.title;
    payload["body"] = in.body;
    payload["deep_link"] = in.deepLink;
    fn(in.userId, payload);
}

void NotifyDispatcher::notify(DbClientPtr db,
                              const NotifyInput& in,
                              std::function<void(NotifyOutcome)> cb)
{
    // Инварианты: deep_link заполняется всегда (02 §4.1), title NOT NULL.
    if (!db || in.userId <= 0 || in.title.empty() || in.deepLink.empty())
    {
        LOG_ERROR << "NotifyDispatcher::notify: отклонён ввод (user "
                  << in.userId << ", type " << notifyTypeName(in.type)
                  << (in.deepLink.empty() ? ", пустой deep_link" : "")
                  << (in.title.empty() ? ", пустой title" : "") << ")";
        cb(NotifyOutcome::Error);
        return;
    }

    const auto now = nowUtc();
    const int minute = minuteOfDayOf(now);

    // Настройки пользователя (тумблеры + quiet_hours_json). load уже
    // fail-closed к дефолтам DDL (Э2 шаг 3); финальную ошибку БД здесь
    // отловит INSERT — исход Error без строки и без push.
    SettingsService::load(db,
                          in.userId,
                          [db, in, cb = std::move(cb), now, minute](
                              SettingsRecord settings) {
                              const bool urgent =
                                  in.type == NotifyType::Schedule &&
                                  isUrgentSchedule(in.eventStartsAtUtc, now);
                              const NotifyOutcome outcome =
                                  decide(settings, in.type, urgent, minute);
                              if (outcome != NotifyOutcome::Sent)
                              {
                                  if (outcome == NotifyOutcome::DeferredToDigest)
                                  {
                                      // Очередь/пометка дайджеста (шаг 2 Э6
                                      // разберёт утром); в БД не пишем.
                                      DeferredNotification d;
                                      d.userId = in.userId;
                                      d.type = in.type;
                                      d.title = in.title;
                                      d.body = in.body;
                                      d.deepLink = in.deepLink;
                                      d.deferredAtUtc = timeToIsoUtc(
                                          std::chrono::system_clock::to_time_t(
                                              now));
                                      std::lock_guard<std::mutex> lock(gMutex);
                                      gPendingDigest.push_back(std::move(d));
                                  }
                                  cb(outcome);
                                  return;
                              }

                              // Sent: строка в notifications — источник
                              // истины (02 §4.5). cb копируется в обе ветки
                              // цепочки (урок Э2: move в success-ветку
                              // оставил бы error-ветке пустой std::function).
                              std::optional<std::string> body;
                              if (!in.body.empty())
                                  body = in.body;
                              *db << kInsertSql << in.userId
                                  << notifyTypeName(in.type) << in.title << body
                                  << in.deepLink >>
                                  [in, cb](const Result&) {
                                      // Пуш — только после записи в БД.
                                      NotifyDispatcher::pushLive(in);
                                      cb(NotifyOutcome::Sent);
                                  } >>
                                  [cb](const DrogonDbException& e) {
                                      LOG_ERROR
                                          << "NotifyDispatcher::notify: insert: "
                                          << e.base().what();
                                      cb(NotifyOutcome::Error);  // fail-closed
                                  };
                          });
}

std::vector<DeferredNotification> NotifyDispatcher::takePendingDigest()
{
    std::lock_guard<std::mutex> lock(gMutex);
    return std::exchange(gPendingDigest, {});
}

std::size_t NotifyDispatcher::pendingDigestCount()
{
    std::lock_guard<std::mutex> lock(gMutex);
    return gPendingDigest.size();
}

void NotifyDispatcher::resetForTests()
{
    std::lock_guard<std::mutex> lock(gMutex);
    gPushFn = nullptr;
    gClock = nullptr;
    gPendingDigest.clear();
}

void NotifyDispatcher::flushDigest(DbClientPtr db, FlushCb cb)
{
    if (!db || !cb)
    {
        if (cb)
            cb(DigestFlush{});
        return;
    }

    // 1. Забрать очередь целиком (доставленные повторно не шлются: элемент
    //    существует ровно в одном экземпляре — либо в очереди, либо уже
    //    отправлен в решение notify()).
    std::vector<DeferredNotification> batch = takePendingDigest();
    if (batch.empty())
    {
        cb(DigestFlush{});
        return;
    }

    // 2. Группировка по владельцу (map — детерминированный порядок доставки).
    std::map<int64_t, std::vector<DeferredNotification>> groups;
    for (auto& item : batch)
        groups[item.userId].push_back(std::move(item));

    auto state = std::make_shared<DigestFlush>();
    auto remainingUsers = std::make_shared<std::atomic<int>>(
        static_cast<int>(groups.size()));
    auto remainingItems = std::make_shared<std::atomic<int>>(
        static_cast<int>(batch.size()));
    auto finished = std::make_shared<std::atomic<bool>>(false);
    auto finish = [state, cb, finished] {
        bool expected = false;
        if (finished->compare_exchange_strong(expected, true))
            cb(*state);
    };
    auto checkDone = [remainingUsers, remainingItems, finish] {
        if (remainingUsers->load() == 0 && remainingItems->load() == 0)
            finish();
    };

    // Тихие часы считаются на момент тика: владелец вне своего интервала →
    // его очередь готова (первый тик после конца его тихих часов).
    const int minute = minuteOfDayOf(nowUtc());

    for (auto& [userId, items] : groups)
    {
        SettingsService::load(
            db,
            userId,
            [db, userId, items = std::move(items), minute, state,
             remainingUsers, remainingItems, checkDone](
                SettingsRecord settings) mutable {
                if (inQuietHours(settings.quietHoursJson, minute))
                {
                    // Владелец ещё в тихих часах: вернуть группу в очередь
                    // (элементы, отложенные параллельно только что, не тронуты).
                    const std::size_t held = items.size();
                    {
                        std::lock_guard<std::mutex> lock(gMutex);
                        for (auto& it : items)
                            gPendingDigest.push_back(std::move(it));
                    }
                    items.clear();
                    state->held += held;
                    remainingItems->fetch_sub(static_cast<int>(held));
                }
                else
                {
                    // Первый тик после конца тихих часов: доставить разом.
                    // Каждый элемент — через notify(): создаст строку и
                    // пуш, применит тумблер (настройки могли измениться
                    // за ночь); тихие часы здесь уже не отложат — тик
                    // проверил интервал владельца. Любой исход = элемент
                    // обработан, повторно не шлётся (удалён из очереди).
                    for (const auto& it : items)
                    {
                        NotifyInput in;
                        in.userId = userId;
                        in.type = it.type;
                        in.title = it.title;
                        in.body = it.body;
                        in.deepLink = it.deepLink;
                        notify(db, in,
                               [state, remainingItems,
                                checkDone](NotifyOutcome) {
                                   state->delivered += 1;
                                   remainingItems->fetch_sub(1);
                                   checkDone();
                               });
                    }
                }
                remainingUsers->fetch_sub(1);
                checkDone();
            });
    }
}

}  // namespace para
