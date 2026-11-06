// Юнит-тесты ReminderService (Э6 шаг 2; гайд BE §9, 02 §4 поток 4,
// миграция 0003: deadlines.reminded, deadline_reminders.remind_at/sent_at):
//  - дедлайн за 24 ч → уведомление type='deadline' участникам + reminded=1;
//  - повторный тик → без дубля (метки — идемпотентность);
//  - персональный remind_at → уведомление + sent_at;
//  - дайджест: доставляет накопленное за ночь и очищает очередь;
//  - границы окна (now+24h включительно, due_at в прошлом/пометка — мимо);
//  - runMinuteTick — конвейер трёх фаз, статистика.
// Временная SQLite через настоящий DbClient (как в тестах Э2–Э5а), время —
// тестовый шов NotifyDispatcher::setClock (единое «сейчас» для окна тика и
// решения о тихих часах), колбэки синхронизируются promise/future.
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <future>
#include <string>
#include <utility>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/NotifyDispatcher.h"
#include "services/ReminderService.h"

namespace fs = std::filesystem;

using para::NotifyDispatcher;
using para::NotifyInput;
using para::NotifyOutcome;
using para::NotifyType;
using para::ReminderService;

// Потоковый вывод enum'ов для CHECK (doctest 2.5.3).
namespace para {
inline std::ostream& operator<<(std::ostream& os, NotifyOutcome outcome)
{
    switch (outcome)
    {
        case NotifyOutcome::Sent:
            return os << "Sent";
        case NotifyOutcome::DeferredToDigest:
            return os << "DeferredToDigest";
        case NotifyOutcome::SuppressedByToggle:
            return os << "SuppressedByToggle";
        case NotifyOutcome::Error:
            return os << "Error";
    }
    return os;
}
}  // namespace para

namespace {

struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-remind-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void exec(sqlite3* db, const std::string& sql)
{
    char* err = nullptr;
    REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
    if (err != nullptr)
    {
        FAIL(err);
        sqlite3_free(err);
    }
}

// Схема — копии колонок миграций 0003 (deadlines, deadline_reminders) и
// 0004 (notifications с CHECK, settings) + profiles (0001/0002, только
// user_id/group_id) — БЕЗ FK (FK проверяют миграции, не юнит-тест).
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    Fixture()
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        exec(raw,
             "CREATE TABLE deadlines ("
             "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  group_id INTEGER,"
             "  user_id  INTEGER,"
             "  title    TEXT NOT NULL,"
             "  kind     TEXT NOT NULL,"
             "  due_at   TEXT NOT NULL,"
             "  reminded INTEGER NOT NULL DEFAULT 0"
             ");");
        exec(raw,
             "CREATE TABLE deadline_reminders ("
             "  user_id     INTEGER NOT NULL,"
             "  deadline_id INTEGER NOT NULL,"
             "  remind_at   TEXT NOT NULL,"
             "  sent_at     TEXT,"
             "  PRIMARY KEY (user_id, deadline_id)"
             ");");
        exec(raw,
             "CREATE TABLE notifications ("
             "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id    INTEGER NOT NULL,"
             "  type       TEXT NOT NULL CHECK (type IN "
             "    ('schedule','deadline','application','announcement')),"
             "  title      TEXT NOT NULL,"
             "  body       TEXT,"
             "  deep_link  TEXT,"
             "  read_at    TEXT,"
             "  created_at TEXT NOT NULL DEFAULT "
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now'))"
             ");");
        exec(raw,
             "CREATE TABLE settings ("
             "  user_id             INTEGER PRIMARY KEY,"
             "  notify_schedule     INTEGER NOT NULL DEFAULT 1,"
             "  notify_deadlines    INTEGER NOT NULL DEFAULT 1,"
             "  notify_applications INTEGER NOT NULL DEFAULT 1,"
             "  quiet_hours_json    TEXT    NOT NULL DEFAULT "
             "'{\"from\":\"23:00\",\"to\":\"07:00\"}',"
             "  theme               TEXT    NOT NULL DEFAULT 'system',"
             "  onboarding_json     TEXT    NOT NULL DEFAULT '[]'"
             ");");
        exec(raw,
             "CREATE TABLE profiles ("
             "  user_id  INTEGER PRIMARY KEY,"
             "  group_id INTEGER"
             ");");
        sqlite3_close(raw);
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }

    void insertRows(const std::string& sql)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READWRITE,
                                nullptr) == SQLITE_OK);
        exec(raw, sql);
        sqlite3_close(raw);
    }
};

// RAII-сброс статиков диспетчера (часы/пуш/очередь) между кейсами.
struct DispatchGuard
{
    DispatchGuard() { NotifyDispatcher::resetForTests(); }
    ~DispatchGuard() { NotifyDispatcher::resetForTests(); }
};

// Тестовое «сейчас»: день 06.11 12:00 UTC — вне тихих часов 23:00–07:00.
const std::string kNowIso = "2026-11-06T12:00:00Z";    // окно до 07.11 12:00
const std::string kNightIso = "2026-11-07T02:00:00Z";  // ночь: внутри 23–07
const std::string kMorningIso = "2026-11-07T07:00:00Z";  // конец тихих часов

std::chrono::system_clock::time_point tpOf(const std::string& iso)
{
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    REQUIRE(sscanf(iso.c_str(),
                   "%d-%d-%dT%d:%d:%d",
                   &y,
                   &mo,
                   &d,
                   &h,
                   &mi,
                   &s) == 6);
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = s;
    tm.tm_isdst = 0;
    return std::chrono::system_clock::from_time_t(timegm(&tm));
}

void setClockTo(const std::string& iso)
{
    NotifyDispatcher::setClock([iso] { return tpOf(iso); });
}

int64_t scalarOf(const std::string& dbPath, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) ==
            SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    const int64_t v = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
    return v;
}

int basicRound(const drogon::orm::DbClientPtr& db)
{
    std::promise<int> p;
    auto f = p.get_future();
    ReminderService::basicDeadlineRound(db, [&p](int n) { p.set_value(n); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

int personalRound(const drogon::orm::DbClientPtr& db)
{
    std::promise<int> p;
    auto f = p.get_future();
    ReminderService::personalReminderRound(db, [&p](int n) { p.set_value(n); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ReminderService::TickStats tickOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<ReminderService::TickStats> p;
    auto f = p.get_future();
    ReminderService::runMinuteTick(db, [&p](ReminderService::TickStats s) {
        p.set_value(s);
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

NotifyOutcome notifyNow(const drogon::orm::DbClientPtr& db, NotifyInput in)
{
    std::promise<NotifyOutcome> p;
    auto f = p.get_future();
    NotifyDispatcher::notify(db,
                             std::move(in),
                             [&p](NotifyOutcome o) { p.set_value(o); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// Фиксатор WS-push (диспетчер пушит после записи строки).
struct PushCapture
{
    std::vector<std::pair<int64_t, Json::Value>> calls;
    NotifyDispatcher::PushFn fn()
    {
        return [this](int64_t userId, const Json::Value& payload) {
            calls.emplace_back(userId, payload);
        };
    }
};

}  // namespace

TEST_CASE("ReminderService: дедлайн за 24 ч — уведомление участникам + метка")
{
    DispatchGuard guard;
    Fixture f;
    setClockTo(kNowIso);
    // Групповой дедлайн группы 1 (Аня и Олег, профили ниже) — через 22 ч;
    // персональный Марка (user 3) — через 18 ч.
    f.insertRows(
        "INSERT INTO profiles (user_id, group_id) VALUES (1, 1), (2, 1);"
        "INSERT INTO deadlines (id, group_id, user_id, title, kind, due_at, "
        "reminded) VALUES "
        "  (1, 1, NULL, 'Эссе по философии', 'work', '2026-11-07T10:00:00Z', 0),"
        "  (2, NULL, 3, 'Отчёт по эконометрике', 'work', "
        "'2026-11-07T06:00:00Z', 0);");
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());

    CHECK(basicRound(f.db) == 2);

    // Уведомления всем трём получателям, type='deadline', deep_link — радар.
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE type='deadline' "
                   "AND deep_link='/deadlines'") == 3);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=1") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=2") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=3") == 1);
    // Метка reminded=1 у обоих (идемпотентность следующего тика).
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadlines "
                             "WHERE reminded=1") == 2);
    // WS-push ушёл по каждому доставленному (тумблеры по умолчанию on).
    CHECK(push.calls.size() == 3);
}

TEST_CASE("ReminderService: повторный тик — без дубля (метка reminded)")
{
    DispatchGuard guard;
    Fixture f;
    setClockTo(kNowIso);
    f.insertRows(
        "INSERT INTO profiles (user_id, group_id) VALUES (1, 1);"
        "INSERT INTO deadlines (id, group_id, user_id, title, kind, due_at, "
        "reminded) VALUES (1, 1, NULL, 'Эссе', 'work', "
        "'2026-11-07T10:00:00Z', 0);");

    CHECK(basicRound(f.db) == 1);
    const auto afterFirst = scalarOf(
        f.dbPath, "SELECT count(*) FROM notifications");
    CHECK(afterFirst == 1);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadlines "
                             "WHERE reminded=1") == 1);

    // Перезапуск приложения / следующий тик: окно то же, но reminded=1 —
    // дедлайн больше не выбирается, дублей нет.
    CHECK(basicRound(f.db) == 0);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") ==
          afterFirst);
}

TEST_CASE("ReminderService: границы окна «за 1 день»")
{
    DispatchGuard guard;
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    setClockTo(kNowIso);  // окно (06.11 12:00; 07.11 12:00]

    SUBCASE("due_at ровно now+24h — входит (граница включена)")
    {
        Fixture f;
        f.insertRows("INSERT INTO deadlines (id, group_id, user_id, title, "
                     "kind, due_at, reminded) VALUES (1, NULL, 3, 'Работа', "
                     "'work', '2026-11-07T12:00:00Z', 0);");
        CHECK(basicRound(f.db) == 1);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
    }
    SUBCASE("due_at = now — не входит (окно строго в будущее)")
    {
        Fixture f;
        f.insertRows("INSERT INTO deadlines (id, group_id, user_id, title, "
                     "kind, due_at, reminded) VALUES (1, NULL, 3, 'Работа', "
                     "'work', '2026-11-06T12:00:00Z', 0);");
        CHECK(basicRound(f.db) == 0);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    }
    SUBCASE("due_at в прошлом — не напоминаем")
    {
        Fixture f;
        f.insertRows("INSERT INTO deadlines (id, group_id, user_id, title, "
                     "kind, due_at, reminded) VALUES (1, NULL, 3, 'Работа', "
                     "'work', '2026-11-05T09:00:00Z', 0);");
        CHECK(basicRound(f.db) == 0);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadlines "
                                 "WHERE reminded=1") == 0);
    }
    SUBCASE("reminded=1 заранее — уже расслан, мимо")
    {
        Fixture f;
        f.insertRows("INSERT INTO deadlines (id, group_id, user_id, title, "
                     "kind, due_at, reminded) VALUES (1, NULL, 3, 'Работа', "
                     "'work', '2026-11-07T09:00:00Z', 1);");
        CHECK(basicRound(f.db) == 0);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    }
}

TEST_CASE("ReminderService: персональный remind_at — уведомление + sent_at")
{
    DispatchGuard guard;
    Fixture f;
    setClockTo(kNowIso);
    f.insertRows(
        "INSERT INTO deadlines (id, group_id, user_id, title, kind, due_at, "
        "reminded) VALUES (10, NULL, 3, 'Семинар по мировой экономике', "
        "'work', '2026-11-12T10:00:00Z', 1);"
        "INSERT INTO deadline_reminders (user_id, deadline_id, remind_at, "
        "sent_at) VALUES (3, 10, '2026-11-06T11:00:00Z', NULL);");

    CHECK(personalRound(f.db) == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=3 AND "
                   "type='deadline' AND deep_link='/deadlines'") == 1);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadline_reminders "
                             "WHERE sent_at IS NOT NULL") == 1);
    // Повторный тик: sent_at проставлен — напоминание не шлётся вновь.
    CHECK(personalRound(f.db) == 0);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);

    SUBCASE("remind_at в будущем — не наступил, мимо")
    {
        Fixture f2;
        f2.insertRows(
            "INSERT INTO deadlines (id, group_id, user_id, title, kind, "
            "due_at, reminded) VALUES (11, NULL, 3, 'Работа', 'work', "
            "'2026-11-20T10:00:00Z', 1);"
            "INSERT INTO deadline_reminders (user_id, deadline_id, remind_at, "
            "sent_at) VALUES (3, 11, '2026-11-06T13:00:00Z', NULL);");
        CHECK(personalRound(f2.db) == 0);
        CHECK(scalarOf(f2.dbPath, "SELECT count(*) FROM notifications") == 0);
    }
}

TEST_CASE("ReminderService: дайджест — доставляет накопленное и очищает")
{
    DispatchGuard guard;
    Fixture f;
    // Ночь: user 1 — дефолтные тихие часы 23:00–07:00 (строка настроек есть,
    // чтобы её можно было править в субкейсе); user 2 — свои 02:00–02:30;
    // оба откладывают уведомления в очередь дайджеста (шаг 1).
    setClockTo(kNightIso);
    f.insertRows(
        "INSERT INTO settings (user_id) VALUES (1);"
        "INSERT INTO settings (user_id, quiet_hours_json) VALUES "
        "  (2, '{\"from\":\"02:00\",\"to\":\"02:30\"}');");
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());

    NotifyInput a = NotifyInput{};
    a.userId = 1;
    a.type = NotifyType::Application;
    a.title = "Справка принята в работу";
    a.deepLink = "/applications/2";
    CHECK(notifyNow(f.db, a) == NotifyOutcome::DeferredToDigest);
    NotifyInput b = NotifyInput{};
    b.userId = 2;
    b.type = NotifyType::Deadline;
    b.title = "Отчёт по эконометрике";
    b.deepLink = "/deadlines";
    CHECK(notifyNow(f.db, b) == NotifyOutcome::DeferredToDigest);
    CHECK(NotifyDispatcher::pendingDigestCount() == 2);

    SUBCASE("ночь: владелец в тихих часах — элемент держится в очереди")
    {
        std::promise<NotifyDispatcher::DigestFlush> p;
        auto fu = p.get_future();
        NotifyDispatcher::flushDigest(f.db, [&p](auto r) { p.set_value(r); });
        REQUIRE(fu.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        const auto r = fu.get();
        CHECK(r.delivered == 0);
        CHECK(r.held == 2);
        CHECK(NotifyDispatcher::pendingDigestCount() == 2);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    }
    SUBCASE("утро (первый тик после конца тихих часов): разом и без повтора")
    {
        setClockTo(kMorningIso);  // 07:00 — вне обоих интервалов
        std::promise<NotifyDispatcher::DigestFlush> p;
        auto fu = p.get_future();
        NotifyDispatcher::flushDigest(f.db, [&p](auto r) { p.set_value(r); });
        REQUIRE(fu.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        const auto r = fu.get();
        CHECK(r.delivered == 2);
        CHECK(r.held == 0);
        CHECK(NotifyDispatcher::pendingDigestCount() == 0);  // очищена
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 2);
        CHECK(push.calls.size() == 2);
        CHECK(push.calls[0].second["deep_link"].asString() ==
              "/applications/2");
        CHECK(push.calls[1].second["type"].asString() == "deadline");

        // Повторный flush: очередь пуста — доставленное повторно не шлётся.
        std::promise<NotifyDispatcher::DigestFlush> p2;
        auto fu2 = p2.get_future();
        NotifyDispatcher::flushDigest(f.db, [&p2](auto r) { p2.set_value(r); });
        REQUIRE(fu2.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        CHECK(fu2.get().delivered == 0);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 2);
    }
    SUBCASE("тумблер выключили за ночь — элемент извлечён и подавлен")
    {
        f.insertRows("UPDATE settings SET notify_applications=0 "
                     "WHERE user_id=1;");
        setClockTo(kMorningIso);
        std::promise<NotifyDispatcher::DigestFlush> p;
        auto fu = p.get_future();
        NotifyDispatcher::flushDigest(f.db, [&p](auto r) { p.set_value(r); });
        REQUIRE(fu.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        const auto r = fu.get();
        CHECK(r.delivered == 2);  // обработаны оба (один подавлен)
        CHECK(r.held == 0);
        // application у 1 подавлен, deadline у 2 доставлен.
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications "
                                 "WHERE user_id=2") == 1);
        CHECK(NotifyDispatcher::pendingDigestCount() == 0);
    }
}

TEST_CASE("ReminderService: runMinuteTick — конвейер фаз и статистика")
{
    DispatchGuard guard;
    Fixture f;
    // Дедлайн (группа 1: Аня и Олег) + готовое персональное напоминание
    // (Марк) + элемент дайджеста Дины, отложенный ночью.
    f.insertRows(
        "INSERT INTO profiles (user_id, group_id) VALUES (1, 1), (2, 1);"
        "INSERT INTO deadlines (id, group_id, user_id, title, kind, due_at, "
        "reminded) VALUES (1, 1, NULL, 'Эссе по философии', 'work', "
        "'2026-11-07T10:00:00Z', 0),"
        "  (10, NULL, 3, 'Семинар', 'work', '2026-11-12T10:00:00Z', 1);"
        "INSERT INTO deadline_reminders (user_id, deadline_id, remind_at, "
        "sent_at) VALUES (3, 10, '2026-11-06T11:00:00Z', NULL);");

    setClockTo(kNightIso);  // ночь: notify Дины откладывается в дайджест
    NotifyInput d = NotifyInput{};
    d.userId = 4;
    d.type = NotifyType::Application;
    d.title = "Справка готова";
    d.deepLink = "/applications/1";
    CHECK(notifyNow(f.db, d) == NotifyOutcome::DeferredToDigest);
    setClockTo(kNowIso);  // день 06.11 12:00: все три фазы готовы

    const auto stats = tickOf(f.db);
    CHECK(stats.deadlineNotices == 1);
    CHECK(stats.personalReminders == 1);
    CHECK(stats.digestDelivered == 1);
    CHECK(stats.digestHeld == 0);
    // 2 (Аня+Олег) + 1 (Марк) + 1 (Дина, из дайджеста) = 4 строки.
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 4);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadlines "
                             "WHERE id=1 AND reminded=1") == 1);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadline_reminders "
                             "WHERE sent_at IS NOT NULL") == 1);
    CHECK(NotifyDispatcher::pendingDigestCount() == 0);

    // Повторный тик: все метки стоят — нули, дублей нет (идемпотентность).
    const auto again = tickOf(f.db);
    CHECK(again.deadlineNotices == 0);
    CHECK(again.personalReminders == 0);
    CHECK(again.digestDelivered == 0);
    CHECK(again.digestHeld == 0);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 4);
}

TEST_CASE("ReminderService: ночная рассылка дедлайна уходит в дайджест")
{
    DispatchGuard guard;
    Fixture f;
    // 02 §4 поток 4: «в обоих случаях применяются тихие часы — ночью копится
    // в утренний дайджест». Тик ночью: дедлайн расслан (reminded=1), но
    // строки уведомлений создаются утренним flush, а не тиком.
    setClockTo(kNightIso);
    f.insertRows(
        "INSERT INTO profiles (user_id, group_id) VALUES (1, 1);"
        "INSERT INTO deadlines (id, group_id, user_id, title, kind, due_at, "
        "reminded) VALUES (1, 1, NULL, 'Эссе по философии', 'work', "
        "'2026-11-07T20:00:00Z', 0);");

    CHECK(basicRound(f.db) == 1);   // рассылка сделана
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM deadlines "
                             "WHERE reminded=1") == 1);
    CHECK(NotifyDispatcher::pendingDigestCount() == 1);  // Аня — в дайджесте

    // Утренний flush доставляет отложенное напоминание.
    setClockTo(kMorningIso);
    std::promise<NotifyDispatcher::DigestFlush> p;
    auto fu = p.get_future();
    NotifyDispatcher::flushDigest(f.db, [&p](auto r) { p.set_value(r); });
    REQUIRE(fu.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    CHECK(fu.get().delivered == 1);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
    // Повторный тик дедлайна не дублирует (reminded=1, очередь пуста).
    CHECK(basicRound(f.db) == 0);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
}
