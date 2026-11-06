// Юнит-тесты staff-части Э6 (шаг 4; гайд BE §9, 02 §5.2 staff-строки):
//  - перенос (ScheduleChangeService::applyChange): prev_* «было → стало»,
//    уведомление type='schedule' КАЖДОМУ студенту группы через NotifyDispatcher
//    с deep_link, инвалидация кэша вызывается, recipients в ответе;
//  - недопустимые действия/тела → 400, чужая группа не получает, повторная
//    отмена → 400, несуществующее событие → 404;
//  - дашборд (StaffDashboard::build): счётчики очереди по 5 статусам +
//    recent_changes;
//  - объявления (AnnouncementsService): аудитория 'all'/массив, видимость по
//    группе/курсу, staff видит всё, публикация без уведомлений, курсор.
// Временная SQLite (схемы 0002/0003/0004 без FK), время — шов NotifyDispatcher,
// колбэки promise/future (паттерн Э2–Э5а).
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <future>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <sqlite3.h>

#include <thread>

#include <doctest.hpp>

#include "services/AnnouncementsService.h"
#include "services/NotifyDispatcher.h"
#include "services/ScheduleChangeService.h"
#include "services/SessionService.h"
#include "services/StaffDashboard.h"

namespace fs = std::filesystem;
using para::AnnouncementsService;
using para::NotifyDispatcher;
using para::NotifyInput;
using para::NotifyOutcome;
using para::ScheduleChangeService;
using para::SessionInfo;
using para::StaffDashboard;

namespace para {
// Потоковый вывод для CHECK (doctest 2.5.3) — как в notify_dispatcher_test.
inline std::ostream& operator<<(std::ostream& os, NotifyOutcome o)
{
    switch (o)
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
               ("para-staff-" + std::to_string(stamp) + "-" +
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

// Схемы миграций 0002 (schedule_events/disciplines/teachers/profiles),
// 0003 (applications/announcements), 0004 (notifications/settings) — без FK.
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
             "CREATE TABLE disciplines (id INTEGER PRIMARY KEY, title TEXT "
             "NOT NULL, code TEXT);"
             "CREATE TABLE teachers (id INTEGER PRIMARY KEY, fio TEXT NOT "
             "NULL);"
             "CREATE TABLE schedule_events ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  group_id INTEGER NOT NULL,"
             "  discipline_id INTEGER NOT NULL,"
             "  teacher_id INTEGER,"
             "  room TEXT NOT NULL,"
             "  kind TEXT NOT NULL,"
             "  starts_at TEXT NOT NULL,"
             "  ends_at TEXT NOT NULL,"
             "  status TEXT NOT NULL DEFAULT 'normal' CHECK (status IN "
             "('normal','moved','cancelled','substituted')),"
             "  prev_room TEXT, prev_starts_at TEXT, prev_teacher_id INTEGER,"
             "  changed_by INTEGER, changed_at TEXT);"
             "CREATE TABLE profiles (user_id INTEGER PRIMARY KEY, fio TEXT, "
             "group_id INTEGER, course INTEGER);"
             "CREATE TABLE applications (id INTEGER PRIMARY KEY "
             "AUTOINCREMENT, user_id INTEGER NOT NULL, type TEXT NOT NULL, "
             "status TEXT NOT NULL, payload_json TEXT, created_at TEXT, "
             "updated_at TEXT);"
             "CREATE TABLE announcements (id INTEGER PRIMARY KEY "
             "AUTOINCREMENT, author_id INTEGER NOT NULL, title TEXT NOT "
             "NULL, body TEXT NOT NULL, audience TEXT NOT NULL DEFAULT "
             "'all', published_at TEXT NOT NULL DEFAULT "
             "(strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE TABLE notifications (id INTEGER PRIMARY KEY "
             "AUTOINCREMENT, user_id INTEGER NOT NULL, type TEXT NOT NULL "
             "CHECK (type IN ('schedule','deadline','application',"
             "'announcement')), title TEXT NOT NULL, body TEXT, deep_link "
             "TEXT, read_at TEXT, created_at TEXT NOT NULL DEFAULT "
             "(strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE TABLE settings (user_id INTEGER PRIMARY KEY, "
             "notify_schedule INTEGER NOT NULL DEFAULT 1, notify_deadlines "
             "INTEGER NOT NULL DEFAULT 1, notify_applications INTEGER NOT "
             "NULL DEFAULT 1, quiet_hours_json TEXT NOT NULL DEFAULT "
             "'{\"from\":\"23:00\",\"to\":\"07:00\"}', theme TEXT NOT NULL "
             "DEFAULT 'system', onboarding_json TEXT NOT NULL DEFAULT '[]');");
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

// RAII-сброс статиков диспетчера (часы/пуш/очередь дайджеста).
struct DispatchGuard
{
    DispatchGuard() { NotifyDispatcher::resetForTests(); }
    ~DispatchGuard() { NotifyDispatcher::resetForTests(); }
};

const std::string kNowIso = "2026-11-06T12:00:00Z";  // день: мимо тихих часов

std::chrono::system_clock::time_point tpOf(const std::string& iso)
{
    int y = 0, mo = 0, d = 0, h = 0, mi = 0, s = 0;
    REQUIRE(sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi,
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

int64_t scalarOf(const std::string& dbPath, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    // Авторассылка фоновая (201 не ждёт INSERT'ов) — читатель ждёт лок
    // писателя, а не падает SQLITE_BUSY.
    sqlite3_busy_timeout(raw, 3000);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) ==
            SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    const int64_t v = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
    return v;
}

// Ожидание асинхронного эффекта (notify-INSERT / WS-push): поллинг до 5 с.
int64_t waitCount(const std::string& dbPath, const std::string& sql, int64_t want)
{
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(5);
    int64_t got = -1;
    while (std::chrono::steady_clock::now() < deadline)
    {
        got = scalarOf(dbPath, sql);
        if (got == want)
            return got;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return got;
}

// Фиксатор WS-push (точка интеграции шага 3 — как в notify_dispatcher_test).
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

void waitPush(const PushCapture& push, std::size_t want)
{
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::seconds(5);
    while (push.calls.size() < want &&
           std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
}

ScheduleChangeService::Result changeOf(
    const drogon::orm::DbClientPtr& db,
    const SessionInfo& staff,
    ScheduleChangeService::Input in,
    ScheduleChangeService::InvalidateFn invalidate)
{
    std::promise<ScheduleChangeService::Result> p;
    auto f = p.get_future();
    ScheduleChangeService::applyChange(db, staff, std::move(in),
                                       std::move(invalidate),
                                       [&p](ScheduleChangeService::Result r) {
                                           p.set_value(std::move(r));
                                       });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

SessionInfo staffSession(int64_t id = 5)
{
    SessionInfo s;
    s.userId = id;
    s.login = "dean.demo";
    s.role = "staff";
    return s;
}




// Фейк-кэш: счётчик вызовов инвалидации по группе.
struct InvalidateSpy
{
    std::vector<int64_t> groups;
    ScheduleChangeService::InvalidateFn fn()
    {
        return [this](int64_t gid) {
            MESSAGE("DEBUG: invalidate called for group ", gid);
            groups.push_back(gid);
        };
    }
};

}  // namespace

TEST_CASE("ScheduleChangeService: перенос — prev_*, рассылка группе, кэш")
{
    DispatchGuard guard;
    Fixture f;
    NotifyDispatcher::setClock([] { return tpOf(kNowIso); });
    // Событие 8 — фикстурное: группа 1, дисциплина 4 «История России»,
    // семинар 05.11 14:00, ауд. 1201 (копия из sql/fixtures.sql).
    f.insertRows(
        "INSERT INTO disciplines (id, title, code) VALUES (4, 'История "
        "России', 'HR-101');"
        "INSERT INTO teachers (id, fio) VALUES (4, 'Симонов Алексей "
        "Владимирович'), (6, 'Егоров Павел Дмитриевич');"
        "INSERT INTO schedule_events (id, group_id, discipline_id, "
        "teacher_id, room, kind, starts_at, ends_at, status) VALUES "
        "(8, 1, 4, 4, '1201', 'seminar', '2026-11-05T14:00:00Z', "
        "'2026-11-05T15:30:00Z', 'normal');"
        "INSERT INTO profiles (user_id, group_id, course) VALUES "
        "(1, 1, 1), (2, 1, 1), (3, 2, 4);");
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    InvalidateSpy spy;

    ScheduleChangeService::Input in;
    in.eventId = 8;
    in.action = ScheduleChangeService::Action::Move;
    in.startsAt = "2026-11-09T14:00:00Z";
    in.room = "4210";
    in.reason = "Перенос на понедельник (вымышленный)";
    const auto r = changeOf(f.db, staffSession(), in, spy.fn());
    CHECK(r.outcome == ScheduleChangeService::Outcome::Ok);

    // Тело 201: событие в формате 02 §5.2 с «было → стало».
    REQUIRE(r.body.isMember("event"));
    CHECK(r.body["action"].asString() == "move");
    CHECK(r.body["event"]["status"].asString() == "moved");
    CHECK(r.body["event"]["starts_at"].asString() == "2026-11-09T14:00:00Z");
    CHECK(r.body["event"]["room"].asString() == "4210");
    // ends_at сдвинулся на дельту переноса (длительность пары сохранена).
    CHECK(r.body["event"]["ends_at"].asString() == "2026-11-09T15:30:00Z");
    CHECK(r.body["event"]["prev_starts_at"].asString() ==
          "2026-11-05T14:00:00Z");
    CHECK(r.body["event"]["prev_room"].asString() == "1201");
    CHECK(r.body["recipients"].asInt64() == 2);  // Аня и Олег (Марк — группа 2)

    // Инвалидация кэша группы вызвана (хук Э3), ровно один раз.
    REQUIRE(spy.groups.size() == 1);
    CHECK(spy.groups[0] == 1);

    // Уведомления КАЖДОМУ студенту группы, type='schedule', deep_link на
    // новый день; Марку (группа 2) — ничего. Рассылка фоновая — ждём.
    CHECK(waitCount(f.dbPath,
                    "SELECT count(*) FROM notifications WHERE "
                    "type='schedule' AND deep_link='/schedule?date=2026-11-09'",
                    2) == 2);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=1") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=2") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE user_id=3") == 0);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE title LIKE "
                   "'Пара перенесена: История России%'") == 2);
    // Причина — в теле уведомления.
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE body LIKE "
                   "'%Перенос на понедельник%'") == 2);
    // WS-push ушёл обоим (день, тумблеры по умолчанию on).
    waitPush(push, 2);
    CHECK(push.calls.size() == 2);

    // Повторное подключение диспетчера не нужно: строка в БД — источник
    // истины; фоновая рассылка не ждёт 201-ответ.
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM schedule_events WHERE id=8 AND "
                   "status='moved' AND changed_at IS NOT NULL") == 1);
}

TEST_CASE("ScheduleChangeService: недопустимые действия и тела — 400/404")
{
    DispatchGuard guard;
    Fixture f;
    NotifyDispatcher::setClock([] { return tpOf(kNowIso); });
    f.insertRows(
        "INSERT INTO disciplines (id, title, code) VALUES (4, 'История "
        "России', 'HR-101');"
        "INSERT INTO teachers (id, fio) VALUES (4, 'Симонов'), "
        "(6, 'Егоров Павел Дмитриевич');"
        "INSERT INTO schedule_events (id, group_id, discipline_id, "
        "teacher_id, room, kind, starts_at, ends_at, status) VALUES "
        "(8, 1, 4, 4, '1201', 'seminar', '2026-11-05T14:00:00Z', "
        "'2026-11-05T15:30:00Z', 'normal'),"
        "(7, 1, 4, 4, '2109', 'lecture', '2026-11-05T10:00:00Z', "
        "'2026-11-05T11:30:00Z', 'cancelled');"
        "INSERT INTO profiles (user_id, group_id) VALUES (1, 1);");

    SUBCASE("move без starts_at и room — 400")
    {
        ScheduleChangeService::Input in;
        in.eventId = 8;
        in.action = ScheduleChangeService::Action::Move;
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::BadRequest);
    }
    SUBCASE("move с некалендарной датой — 400")
    {
        ScheduleChangeService::Input in;
        in.eventId = 8;
        in.action = ScheduleChangeService::Action::Move;
        in.startsAt = "2026-02-30T14:00:00Z";
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::BadRequest);
    }
    SUBCASE("повторная отмена — 400, состояние не тронуто")
    {
        ScheduleChangeService::Input in;
        in.eventId = 7;
        in.action = ScheduleChangeService::Action::Cancel;
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::BadRequest);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM schedule_events "
                                 "WHERE changed_at IS NOT NULL") == 0);
    }
    SUBCASE("substitute без teacher_id — 400")
    {
        ScheduleChangeService::Input in;
        in.eventId = 8;
        in.action = ScheduleChangeService::Action::Substitute;
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::BadRequest);
    }
    SUBCASE("substitute несуществующего преподавателя — 400")
    {
        ScheduleChangeService::Input in;
        in.eventId = 8;
        in.action = ScheduleChangeService::Action::Substitute;
        in.teacherId = 999;
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::BadRequest);
    }
    SUBCASE("несуществующее событие — 404")
    {
        ScheduleChangeService::Input in;
        in.eventId = 999;
        in.action = ScheduleChangeService::Action::Cancel;
        CHECK(changeOf(f.db, staffSession(), in, nullptr).outcome ==
              ScheduleChangeService::Outcome::NotFound);
    }
    SUBCASE("словарь action — move/cancel/substitute, мусор нет")
    {
        CHECK(ScheduleChangeService::actionFromString("move").has_value());
        CHECK(ScheduleChangeService::actionFromString("cancel").has_value());
        CHECK(
            ScheduleChangeService::actionFromString("substitute").has_value());
        CHECK_FALSE(ScheduleChangeService::actionFromString("destroy")
                        .has_value());
    }
    SUBCASE("замена преподавателя — prev_teacher_id, статус substituted")
    {
        ScheduleChangeService::Input in;
        in.eventId = 8;
        in.action = ScheduleChangeService::Action::Substitute;
        in.teacherId = 6;
        in.reason = "Болезнь (вымышленная)";
        const auto r = changeOf(f.db, staffSession(), in, nullptr);
        CHECK(r.outcome == ScheduleChangeService::Outcome::Ok);
        CHECK(r.body["event"]["status"].asString() == "substituted");
        CHECK(r.body["event"]["prev_teacher_id"].asInt64() == 4);
        CHECK(r.body["event"]["starts_at"].asString() ==
              "2026-11-05T14:00:00Z");  // время не менялось
        // Уведомление — про замену, deep_link на прежний день (фоновая
        // рассылка — ждём).
        CHECK(waitCount(f.dbPath,
                        "SELECT count(*) FROM notifications WHERE title LIKE "
                        "'Замена преподавателя: История России%' AND "
                        "deep_link='/schedule?date=2026-11-05'",
                        1) == 1);
    }
}

TEST_CASE("StaffDashboard: счётчики очереди и свежие изменения")
{
    DispatchGuard guard;
    Fixture f;
    NotifyDispatcher::setClock([] { return tpOf(kNowIso); });
    f.insertRows(
        "INSERT INTO applications (id, user_id, type, status) VALUES "
        "  (1, 2, 'study_certificate', 'submitted'),"
        "  (2, 2, 'study_certificate', 'submitted'),"
        "  (3, 4, 'academic_certificate', 'accepted'),"
        "  (4, 3, 'thesis_topic', 'draft'),"  // вне очереди
        "  (5, 4, 'study_certificate', 'in_progress'),"
        "  (6, 3, 'period_certificate', 'issued'),"  // вне очереди
        "  (7, 2, 'study_certificate', 'ready');"
        "INSERT INTO disciplines (id, title, code) VALUES (4, 'История "
        "России', 'HR-101');"
        "INSERT INTO schedule_events (id, group_id, discipline_id, room, "
        "kind, starts_at, ends_at, status, prev_room, changed_at) VALUES "
        "  (5, 1, 4, '4210', 'lecture', '2026-11-04T10:00:00Z', "
        "'2026-11-04T11:30:00Z', 'moved', '3024', '2026-11-02T15:40:00Z'),"
        "  (7, 1, 4, '2109', 'lecture', '2026-11-05T10:00:00Z', "
        "'2026-11-05T11:30:00Z', 'cancelled', '2109', "
        "'2026-11-02T15:45:00Z');");

    std::promise<StaffDashboard::Result> p;
    auto fu = p.get_future();
    StaffDashboard::build(f.db, [&p](StaffDashboard::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(fu.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto r = fu.get();
    REQUIRE(r.ok);

    // Счётчики очереди по 5 активным статусам; draft/issued — вне.
    const auto& queue = r.body["queue"];
    CHECK(queue["submitted"].asInt64() == 2);
    CHECK(queue["accepted"].asInt64() == 1);
    CHECK(queue["in_progress"].asInt64() == 1);
    CHECK(queue["need_info"].asInt64() == 0);
    CHECK(queue["ready"].asInt64() == 1);

    // Свежие изменения расписания: status != 'normal', новые сверху.
    const auto& changes = r.body["recent_changes"];
    REQUIRE(changes.size() == 2);
    CHECK(changes[0]["id"].asInt64() == 7);  // changed_at 15:45 позже 15:40
    CHECK(changes[0]["prev_room"].asString() == "2109");
    CHECK(changes[1]["id"].asInt64() == 5);
    CHECK(changes[1]["prev_room"].asString() == "3024");
}

TEST_CASE("AnnouncementsService: аудитория, лента, публикация без уведомлений")
{
    DispatchGuard guard;
    Fixture f;
    f.insertRows(
        "INSERT INTO profiles (user_id, group_id, course) VALUES "
        "(1, 1, 1), (3, 2, 4), (4, 3, 4);");

    // Публикация staff: 'all', группе 1, курсу 4.
    AnnouncementsService::Audience all;
    all.all = true;
    AnnouncementsService::Audience group1;
    group1.all = false;
    group1.targets.emplace_back("group", 1);
    AnnouncementsService::Audience course4;
    course4.all = false;
    course4.targets.emplace_back("course", 4);

    auto publish = [&](const char* title, const AnnouncementsService::Audience& a)
        -> AnnouncementsService::Result {
        std::promise<AnnouncementsService::Result> p;
        auto fu = p.get_future();
        AnnouncementsService::publish(f.db, 5, title, "Текст объявления (вымышленный)", a,
                                      [&p](AnnouncementsService::Result r) {
                                          p.set_value(std::move(r));
                                      });
        REQUIRE(fu.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        return fu.get();
    };
    CHECK(publish("Объявление всем", all).ok);
    CHECK(publish("Объявление 1-ФМО", group1).ok);
    CHECK(publish("Курс 4", course4).ok);
    // Публикация НЕ создаёт уведомлений (01 §4.5: без push и без ленты-письма).
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);

    // Лента Ани (группа 1, курс 1): 'all' + group:1; курс 4 и группа 2 — мимо.
    SessionInfo anya;
    anya.userId = 1;
    anya.login = "anya.demo";
    anya.role = "student";
    std::promise<AnnouncementsService::Result> p;
    auto fu = p.get_future();
    AnnouncementsService::list(f.db, anya, std::nullopt,
                               [&p](AnnouncementsService::Result r) {
                                   p.set_value(std::move(r));
                               });
    REQUIRE(fu.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto feedAnya = fu.get();
    REQUIRE(feedAnya.ok);
    CHECK(feedAnya.body["announcements"].size() == 2);
    CHECK(feedAnya.body["announcements"][0]["title"].asString() ==
          "Объявление 1-ФМО");  // новые сверху (id DESC)

    // Марк (группа 2, курс 4): 'all' + course:4.
    SessionInfo mark;
    mark.userId = 3;
    mark.login = "mark.demo";
    mark.role = "student";
    std::promise<AnnouncementsService::Result> p2;
    auto fu2 = p2.get_future();
    AnnouncementsService::list(f.db, mark, std::nullopt,
                               [&p2](AnnouncementsService::Result r) {
                                   p2.set_value(std::move(r));
                               });
    REQUIRE(fu2.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto feedMark = fu2.get();
    REQUIRE(feedMark.ok);
    CHECK(feedMark.body["announcements"].size() == 2);
    CHECK(feedMark.body["announcements"][0]["title"].asString() == "Курс 4");

    // staff видит все три.
    SessionInfo dean = staffSession();
    std::promise<AnnouncementsService::Result> p3;
    auto fu3 = p3.get_future();
    AnnouncementsService::list(f.db, dean, std::nullopt,
                               [&p3](AnnouncementsService::Result r) {
                                   p3.set_value(std::move(r));
                               });
    REQUIRE(fu3.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto feedDean = fu3.get();
    REQUIRE(feedDean.ok);
    CHECK(feedDean.body["announcements"].size() == 3);
    // staff-автор без профиля (Р-6): поле author в ответе отсутствует.
    CHECK_FALSE(
        feedDean.body["announcements"][0].isMember("author"));

    // Курсор: первые 2 объявления Ани + next_cursor на третье.
    std::promise<AnnouncementsService::Result> p4;
    auto fu4 = p4.get_future();
    AnnouncementsService::list(f.db, dean, std::optional<int64_t>(feedDean.body["announcements"][1]["id"].asInt64()),
                               [&p4](AnnouncementsService::Result r) {
                                   p4.set_value(std::move(r));
                               });
    REQUIRE(fu4.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto page2 = fu4.get();
    REQUIRE(page2.ok);
    CHECK(page2.body["announcements"].size() == 1);
    CHECK(page2.body["next_cursor"].isNull());

    // Разбор аудитории: канон и ошибки.
    AnnouncementsService::Audience parsed;
    CHECK(AnnouncementsService::parseAudience("all", parsed));
    CHECK(AnnouncementsService::parseAudience(R"(["group:1","course:4"])",
                                              parsed));
    CHECK(parsed.targets.size() == 2);
    CHECK_FALSE(AnnouncementsService::parseAudience(R"(["dept:9"])", parsed));
    CHECK_FALSE(AnnouncementsService::parseAudience(R"(["group:x"])", parsed));
    CHECK_FALSE(AnnouncementsService::parseAudience("[]", parsed));
    CHECK_FALSE(AnnouncementsService::parseAudience("не-json", parsed));
}
