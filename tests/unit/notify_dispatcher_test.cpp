// Юнит-тесты NotifyDispatcher (Э6 шаг 1; 02 §4.5 — единая точка уведомлений,
// 02 §5.6/01 §5.6 — тихие часы, 01 §4.5 — тумблеры и объявления без push):
//  - inQuietHours: дефолт DDL 23:00–07:00 через полночь, односуточный
//    интервал, синонимы start/end, невалидные значения → интервал пуст;
//  - isUrgentSchedule: окно 24 часов (граница включена), прошлое/
//    нечитаемое/некалендарное/отсутствие starts_at → не срочное;
//  - decide (чистая): три тумблера, announcement мимо тихих часов;
//  - notify e2e (временная SQLite): тумблер off → записи нет; тумблер on →
//    строка с deep_link + WS-push; ночное → DeferredToDigest (в БД нет, в
//    очереди дайджеста); срочное ночью → Sent; announcement ночью → Sent
//    без push; пустой deep_link → Error; ошибка БД → fail-closed Error.
// Временная SQLite через настоящий DbClient — как в тестах Э2–Э5а
// (settings_service_test.cpp): схема копирует миграцию 0004 (с CHECK-словарём
// типов), колбэки синхронизируются promise/future, время — тестовым швом
// setClock (в приложении не устанавливается).
#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <future>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/NotifyDispatcher.h"

// Потоковый вывод enum'ов — doctest печатает сравнения CHECK'ов; без него
// разложение выражения не компилируется (doctest 2.5.3, 02 §9.1).
namespace para {
inline std::ostream& operator<<(std::ostream& os, NotifyType type)
{
    return os << notifyTypeName(type);
}
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

namespace fs = std::filesystem;
using para::DeferredNotification;
using para::NotifyDispatcher;
using para::NotifyInput;
using para::NotifyOutcome;
using para::NotifyType;
using para::SettingsRecord;

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
               ("para-notify-" + std::to_string(stamp) + "-" +
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

// Схема — миграция 0004 (02 §4.1), таблицы notifications и settings,
// БЕЗ FK (FK проверяют миграции, не юнит-тест). CHECK-словарь типов
// сохранён: тест ловит запись типа вне словаря на уровне БД.
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withNotifications = true)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        if (withNotifications)
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
             "  onboarding_json     TEXT    NOT NULL DEFAULT '[]',"
             "  lang                TEXT    NOT NULL DEFAULT 'ru' "
             "CHECK (lang IN ('ru','en','zh'))"
             ");");
        sqlite3_close(raw);
        // FK на каждом соединении (Э8 шаг 5, находка Э1) — те же
        // initialization_queries, что в проде (config.json); Drogon
        // 1.9.14 исполняет их на каждом новом соединении пула.
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }

    void insertSettings(int64_t userId,
                        int schedule = 1,
                        int deadlines = 1,
                        int applications = 1,
                        const std::string& quiet =
                            R"({"from":"23:00","to":"07:00"})")
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READWRITE,
                                nullptr) == SQLITE_OK);
        sqlite3_stmt* stmt = nullptr;
        REQUIRE(sqlite3_prepare_v2(
                    raw,
                    "INSERT INTO settings (user_id, notify_schedule, "
                    "notify_deadlines, notify_applications, quiet_hours_json) "
                    "VALUES (?, ?, ?, ?, ?)",
                    -1,
                    &stmt,
                    nullptr) == SQLITE_OK);
        sqlite3_bind_int64(stmt, 1, userId);
        sqlite3_bind_int(stmt, 2, schedule);
        sqlite3_bind_int(stmt, 3, deadlines);
        sqlite3_bind_int(stmt, 4, applications);
        sqlite3_bind_text(stmt, 5, quiet.c_str(), -1, SQLITE_TRANSIENT);
        REQUIRE(sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        sqlite3_close(raw);
    }
};

// RAII-сброс статиков диспетчера (живут в одном бинарнике на все TEST_CASE).
struct DispatchGuard
{
    DispatchGuard() { NotifyDispatcher::resetForTests(); }
    ~DispatchGuard() { NotifyDispatcher::resetForTests(); }
};

// Тестовое «сейчас»: только для констант из этого файла (формат DDL).
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

// День (12:00 UTC — вне тихих часов) и ночь (23:30 UTC — внутри 23:00–07:00).
const std::string kDayIso = "2026-11-07T12:00:00Z";    // минута 720
const std::string kNightIso = "2026-11-07T23:30:00Z";  // минута 1410

NotifyOutcome notifyOf(const drogon::orm::DbClientPtr& db, NotifyInput in)
{
    std::promise<NotifyOutcome> p;
    auto f = p.get_future();
    NotifyDispatcher::notify(db,
                             std::move(in),
                             [&p](NotifyOutcome o) { p.set_value(o); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// Проверка БД в обход клиента-фикстуры (как в settings_service_test).
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

struct NotificationRow
{
    std::string type;
    std::string title;
    std::optional<std::string> body;
    std::optional<std::string> deepLink;
    std::optional<std::string> readAt;
};

std::optional<NotificationRow> firstNotification(const std::string& dbPath)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw,
                               "SELECT type, title, body, deep_link, read_at "
                               "FROM notifications ORDER BY id",
                               -1,
                               &stmt,
                               nullptr) == SQLITE_OK);
    std::optional<NotificationRow> row;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        NotificationRow r;
        r.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        r.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL)
            r.body =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL)
            r.deepLink =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL)
            r.readAt =
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        row = std::move(r);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
    return row;
}

// Фиксатор WS-push (точка интеграции шага 3: реестра ещё нет — сюда должно
// приходить ровно то, что диспетчер пушит живому соединению).
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

NotifyInput inputOf(int64_t userId, NotifyType type)
{
    NotifyInput in;
    in.userId = userId;
    in.type = type;
    in.title = "Заголовок";
    in.body = "Тело уведомления";
    in.deepLink = "/schedule?date=2026-11-04";
    return in;
}

}  // namespace

TEST_CASE("NotifyDispatcher: тихие часы — интервалы и невалидные значения")
{
    // Дефолт DDL 0004: {"from":"23:00","to":"07:00"} — через полночь.
    const std::string def = R"({"from":"23:00","to":"07:00"})";
    CHECK(NotifyDispatcher::inQuietHours(def, 23 * 60 + 5));    // 23:05
    CHECK(NotifyDispatcher::inQuietHours(def, 0));              // полночь
    CHECK(NotifyDispatcher::inQuietHours(def, 6 * 60 + 59));    // 06:59
    CHECK_FALSE(NotifyDispatcher::inQuietHours(def, 7 * 60));   // 07:00 — [to)
    CHECK_FALSE(NotifyDispatcher::inQuietHours(def, 22 * 60 + 59));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(def, 12 * 60));

    // Односуточный интервал [01:00, 05:00).
    const std::string day = R"({"from":"01:00","to":"05:00"})";
    CHECK(NotifyDispatcher::inQuietHours(day, 2 * 60));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(day, 5 * 60));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(day, 0));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(day, 5 * 60 + 30));

    // Синонимы start/end (02 §5.6 называет {start,end}).
    const std::string se = R"({"start":"22:00","end":"08:00"})";
    CHECK(NotifyDispatcher::inQuietHours(se, 22 * 60));
    CHECK(NotifyDispatcher::inQuietHours(se, 7 * 60 + 59));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(se, 8 * 60));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(se, 21 * 60 + 59));

    // Нечитаемое/вырожденное — интервал пуст (не задерживаем).
    CHECK_FALSE(NotifyDispatcher::inQuietHours("{не-json", 1385));
    CHECK_FALSE(NotifyDispatcher::inQuietHours("[]", 1385));
    CHECK_FALSE(NotifyDispatcher::inQuietHours("{}", 1385));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(
        R"({"from":"25:00","to":"07:00"})", 1385));
    CHECK_FALSE(NotifyDispatcher::inQuietHours(
        R"({"from":"23:00","to":"23:00"})", 1385));  // from == to — пусто
}

TEST_CASE("NotifyDispatcher: срочность — событие начинается в течение 24 часов")
{
    const auto noon = tpOf(kDayIso);
    // +3 часа — срочный перенос ближайшей пары.
    CHECK(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-11-07T15:00:00Z"), noon));
    // Граница включена: ровно +24 часа — ещё срочно.
    CHECK(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-11-08T12:00:00Z"), noon));
    // За окном, в прошлом, без даты, нечитаемое, некалендарное — не срочно.
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-11-08T12:00:01Z"), noon));
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-11-09T00:00:00Z"), noon));
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-11-07T10:00:00Z"), noon));
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(std::nullopt, noon));
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("нет-даты"), noon));
    CHECK_FALSE(NotifyDispatcher::isUrgentSchedule(
        std::optional<std::string>("2026-02-30T10:00:00Z"), noon));
}

TEST_CASE("NotifyDispatcher: decide — тумблеры, announcement, тихие часы")
{
    SettingsRecord on;  // дефолты DDL: все тумблеры включены
    const int night = 23 * 60 + 30;
    const int day = 12 * 60;

    SUBCASE("каждый из трёх тумблеров выключен — свой тип подавлен")
    {
        SettingsRecord s = on;
        s.notifySchedule = false;
        CHECK(NotifyDispatcher::decide(s, NotifyType::Schedule, false, day) ==
              NotifyOutcome::SuppressedByToggle);
        s = on;
        s.notifyDeadlines = false;
        CHECK(NotifyDispatcher::decide(s, NotifyType::Deadline, false, day) ==
              NotifyOutcome::SuppressedByToggle);
        s = on;
        s.notifyApplications = false;
        CHECK(NotifyDispatcher::decide(s, NotifyType::Application, false,
                                       day) ==
              NotifyOutcome::SuppressedByToggle);
    }
    SUBCASE("ночное не-срочное → дайджест; срочное → сразу")
    {
        CHECK(NotifyDispatcher::decide(on, NotifyType::Schedule, false,
                                       night) == NotifyOutcome::DeferredToDigest);
        CHECK(NotifyDispatcher::decide(on, NotifyType::Schedule, true, night) ==
              NotifyOutcome::Sent);
        CHECK(NotifyDispatcher::decide(on, NotifyType::Deadline, false,
                                       night) ==
              NotifyOutcome::DeferredToDigest);  // дедлайн не срочный
        CHECK(NotifyDispatcher::decide(on, NotifyType::Application, false,
                                       night) ==
              NotifyOutcome::DeferredToDigest);
    }
    SUBCASE("день — всё создаётся сразу")
    {
        CHECK(NotifyDispatcher::decide(on, NotifyType::Schedule, false, day) ==
              NotifyOutcome::Sent);
    }
    SUBCASE("announcement — без тумблера и мимо тихих часов (01 §5.6)")
    {
        SettingsRecord allOff = on;
        allOff.notifySchedule = false;
        allOff.notifyDeadlines = false;
        allOff.notifyApplications = false;
        CHECK(NotifyDispatcher::decide(allOff, NotifyType::Announcement, false,
                                       night) == NotifyOutcome::Sent);
    }
}

TEST_CASE("NotifyDispatcher::notify: тумблер выключен — записи нет")
{
    DispatchGuard guard;
    Fixture f;
    // user 7: все три тумблера выключены; user 8: выключен только deadlines.
    f.insertSettings(7, 0, 0, 0);
    f.insertSettings(8, 1, 0, 1);
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kDayIso); });

    CHECK(notifyOf(f.db, inputOf(7, NotifyType::Schedule)) ==
          NotifyOutcome::SuppressedByToggle);
    CHECK(notifyOf(f.db, inputOf(7, NotifyType::Deadline)) ==
          NotifyOutcome::SuppressedByToggle);
    CHECK(notifyOf(f.db, inputOf(7, NotifyType::Application)) ==
          NotifyOutcome::SuppressedByToggle);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    CHECK(push.calls.empty());  // подавленное не пушится

    // Тумблер покрывает только свой тип: schedule у user 8 доставляется.
    CHECK(notifyOf(f.db, inputOf(8, NotifyType::Deadline)) ==
          NotifyOutcome::SuppressedByToggle);
    CHECK(notifyOf(f.db, inputOf(8, NotifyType::Schedule)) ==
          NotifyOutcome::Sent);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
}

TEST_CASE(
    "NotifyDispatcher::notify: тумблер on — строка с deep_link и WS-push")
{
    DispatchGuard guard;
    Fixture f;
    // Строки settings нет: load отдаёт дефолты DDL (все тумблеры on).
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kDayIso); });

    NotifyInput in = inputOf(1, NotifyType::Schedule);
    in.title = "Пара в 10:00 перенесена в ауд. 4210";
    in.body = "Мировая экономика, 4 ноября: ауд. 3024 → 4210";
    in.deepLink = "/schedule?date=2026-11-04";
    in.eventStartsAtUtc = std::optional<std::string>("2026-11-07T15:00:00Z");
    CHECK(notifyOf(f.db, in) == NotifyOutcome::Sent);

    const auto row = firstNotification(f.dbPath);
    REQUIRE(row.has_value());
    CHECK(row->type == "schedule");
    CHECK(row->title == in.title);
    REQUIRE(row->body.has_value());
    CHECK(*row->body == in.body);
    // deep_link заполняется всегда (02 §4.1) — тап ведёт к объекту.
    REQUIRE(row->deepLink.has_value());
    CHECK(*row->deepLink == "/schedule?date=2026-11-04");
    CHECK_FALSE(row->readAt.has_value());  // непрочитанное (read_at NULL)
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM notifications WHERE deep_link "
                   "IS NOT NULL AND deep_link <> ''") == 1);

    // WS-push живому соединению — ровно после записи в БД (шаг 3 подключит
    // реестр: сейчас колбэк и есть точка интеграции).
    REQUIRE(push.calls.size() == 1);
    CHECK(push.calls[0].first == 1);
    CHECK(push.calls[0].second["type"].asString() == "schedule");
    CHECK(push.calls[0].second["title"].asString() == in.title);
    CHECK(push.calls[0].second["deep_link"].asString() == in.deepLink);

    // Без eventStartsAtUtc тоже доставляется (день, тихие часы не активны).
    NotifyInput plain = inputOf(1, NotifyType::Deadline);
    plain.deepLink = "/dashboard";
    CHECK(notifyOf(f.db, plain) == NotifyOutcome::Sent);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 2);
    CHECK(push.calls.size() == 2);
}

TEST_CASE("NotifyDispatcher::notify: ночное — не сразу, в очередь дайджеста")
{
    DispatchGuard guard;
    Fixture f;
    f.insertSettings(1);  // тихие часы по умолчанию 23:00–07:00
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kNightIso); });  // 23:30 UTC

    NotifyInput in = inputOf(1, NotifyType::Schedule);
    in.eventStartsAtUtc = std::optional<std::string>("2026-11-09T05:30:00Z");
    CHECK(notifyOf(f.db, in) == NotifyOutcome::DeferredToDigest);

    // НЕ создано сразу: ни строки, ни push (источник истины — только для
    // доставленного; дайджест шаг 2 создаст утром).
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
    CHECK(push.calls.empty());

    // Очередь/пометка: элемент с полным набором полей, включая deep_link.
    CHECK(NotifyDispatcher::pendingDigestCount() == 1);
    auto pending = NotifyDispatcher::takePendingDigest();
    REQUIRE(pending.size() == 1);
    CHECK(pending[0].userId == 1);
    CHECK(pending[0].type == NotifyType::Schedule);
    CHECK(pending[0].title == in.title);
    CHECK(pending[0].deepLink == "/schedule?date=2026-11-04");
    CHECK(pending[0].deferredAtUtc == kNightIso);
    CHECK(NotifyDispatcher::pendingDigestCount() == 0);  // take забрал
}

TEST_CASE("NotifyDispatcher::notify: срочный перенос ночью — сразу")
{
    DispatchGuard guard;
    Fixture f;
    f.insertSettings(1);
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kNightIso); });

    // Событие начинается через 3 часа (ближайшая пара) — тихие часы не
    // задерживают: строка сразу, push сразу, очередь пуста (01 §5.6).
    NotifyInput in = inputOf(1, NotifyType::Schedule);
    in.title = "Пара в 02:30 перенесена в ауд. 4210";
    in.eventStartsAtUtc = std::optional<std::string>("2026-11-08T02:30:00Z");
    CHECK(notifyOf(f.db, in) == NotifyOutcome::Sent);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
    REQUIRE(push.calls.size() == 1);
    CHECK(push.calls[0].first == 1);
    CHECK(NotifyDispatcher::pendingDigestCount() == 0);
}

TEST_CASE("NotifyDispatcher::notify: announcement ночью — сразу и без push")
{
    DispatchGuard guard;
    Fixture f;
    f.insertSettings(1);
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kNightIso); });

    // Объявления push-ем не отправляются и в тихие часы не попадают
    // (01 §4.5/§5.6): строка создаётся сразу, WS-push не стреляет.
    NotifyInput in = inputOf(1, NotifyType::Announcement);
    in.title = "Объявление деканата";
    in.deepLink = "/notifications";
    CHECK(notifyOf(f.db, in) == NotifyOutcome::Sent);
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 1);
    CHECK(push.calls.empty());
    CHECK(NotifyDispatcher::pendingDigestCount() == 0);
}

TEST_CASE("NotifyDispatcher::notify: неверный ввод и ошибка БД — fail-closed")
{
    DispatchGuard guard;
    PushCapture push;
    NotifyDispatcher::setPushFn(push.fn());
    NotifyDispatcher::setClock([] { return tpOf(kDayIso); });

    SUBCASE("пустой deep_link → Error, ничего не записано")
    {
        Fixture f;
        NotifyInput in = inputOf(1, NotifyType::Schedule);
        in.deepLink = "";
        CHECK(notifyOf(f.db, in) == NotifyOutcome::Error);
        CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM notifications") == 0);
        CHECK(push.calls.empty());
    }
    SUBCASE("таблицы notifications нет → Error, без push (fail-closed)")
    {
        Fixture broken(/*withNotifications=*/false);
        CHECK(notifyOf(broken.db, inputOf(1, NotifyType::Schedule)) ==
              NotifyOutcome::Error);
        CHECK(push.calls.empty());
        CHECK(NotifyDispatcher::pendingDigestCount() == 0);
    }
}
