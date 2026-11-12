// Юнит-тесты SettingsService (Э2 шаг 3; 02 §5.2, миграции 0004 + 0008):
//  - дефолты при отсутствии строки (значения DEFAULT из DDL);
//  - UPSERT перезаписывает существующую строку;
//  - сохранение настроек одного пользователя не трогает чужую строку;
//  - applyPatch: theme только light/dark/system, lang только ru/en/zh
//    (5.18), *_json — валидный JSON, отсутствующие поля не меняются,
//    user_id из тела игнорируется. 5.18 шаг 7: каждая невалидная форма
//    lang ('de'/пустое/число/зона/регистр/хвостовой пробел) — единый
//    400-контракт 02 §5.1 (errorCode «bad_request»).
// Временная SQLite через настоящий DbClient — как тесты Э1 и guard'а
// (migration_runner_test.cpp, ownership_guard_test.cpp): схема копирует
// колонки settings из миграций 0004+0008 БЕЗ FK (FK-ссылку на users тест
// не проверяет — это работа миграций), каждый вызов синхронизируется
// promise/future (колбэки DrogonDbClient — с внутреннего потока).
#include <chrono>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/SettingsService.h"

namespace fs = std::filesystem;
using para::SettingsRecord;
using para::SettingsService;

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
               ("para-settings-" + std::to_string(stamp) + "-" +
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

// Схема таблицы settings — колонки из миграций 0004 + 0008/lang (без FK
// на users).
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
};

SettingsRecord loadOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<SettingsRecord> p;
    auto f = p.get_future();
    SettingsService::load(db, userId,
                          [&p](SettingsRecord r) { p.set_value(std::move(r)); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

bool saveNow(const drogon::orm::DbClientPtr& db, const SettingsRecord& rec)
{
    std::promise<bool> p;
    auto f = p.get_future();
    SettingsService::save(db, rec, [&p](bool ok) { p.set_value(ok); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

SettingsService::PatchResult patchOf(const SettingsRecord& base,
                                     const Json::Value& body,
                                     SettingsRecord& out)
{
    return SettingsService::applyPatch(base, body, out);
}

}  // namespace

TEST_CASE("SettingsService: дефолты при отсутствии строки (DEFAULT из DDL)")
{
    Fixture f;
    const auto rec = loadOf(f.db, 42);
    CHECK(rec.userId == 42);
    CHECK(rec.notifySchedule);
    CHECK(rec.notifyDeadlines);
    CHECK(rec.notifyApplications);
    CHECK(rec.quietHoursJson == "{\"from\":\"23:00\",\"to\":\"07:00\"}");
    CHECK(rec.theme == "system");
    CHECK(rec.onboardingJson == "[]");
    CHECK(rec.lang == "ru");  // 5.18: DEFAULT 'ru' из 0008 — русский дефолт
    // строка в БД не создаётся чтением — ленивое создание при первом PUT
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(f.dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, "SELECT count(*) FROM settings", -1,
                               &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    CHECK(sqlite3_column_int(stmt, 0) == 0);
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
}

TEST_CASE("SettingsService: UPSERT перезаписывает строку (без дублей)")
{
    Fixture f;
    auto rec = loadOf(f.db, 1);  // дефолты
    rec.notifyDeadlines = false;
    rec.theme = "dark";
    rec.onboardingJson = R"(["welcome","schedule"])";
    rec.lang = "zh";  // 5.18: выбор сегмента языка — та же UPSERT-строка
    CHECK(saveNow(f.db, rec));

    auto after = loadOf(f.db, 1);
    CHECK(after.notifySchedule);       // не менялось
    CHECK_FALSE(after.notifyDeadlines);
    CHECK(after.theme == "dark");
    CHECK(after.onboardingJson == R"(["welcome","schedule"])");
    CHECK(after.lang == "zh");

    // повторный PUT той же сессии — строка по-прежнему одна
    after.theme = "light";
    CHECK(saveNow(f.db, after));
    auto again = loadOf(f.db, 1);
    CHECK(again.theme == "light");
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(f.dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, "SELECT count(*) FROM settings", -1,
                               &stmt, nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    CHECK(sqlite3_column_int(stmt, 0) == 1);
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
}

TEST_CASE("SettingsService: чужая строка не затрагивается (изоляция 02 §6.3)")
{
    Fixture f;
    auto rec1 = loadOf(f.db, 1);
    rec1.theme = "dark";
    CHECK(saveNow(f.db, rec1));
    auto rec2 = loadOf(f.db, 2);
    rec2.notifySchedule = false;
    CHECK(saveNow(f.db, rec2));

    // user 1 меняет свои настройки — user 2 остаётся прежним
    auto again1 = loadOf(f.db, 1);
    again1.theme = "light";
    again1.notifySchedule = false;
    CHECK(saveNow(f.db, again1));

    const auto check2 = loadOf(f.db, 2);
    CHECK(check2.notifySchedule == false);  // своё значение user 2
    CHECK(check2.notifyDeadlines);          // дефолт user 2 не тронут
    CHECK(check2.theme == "system");        // дефолт user 2 не тронут
    CHECK(check2.theme != "light");
}

TEST_CASE("SettingsService: applyPatch — валидация и игнор user_id")
{
    Fixture f;
    const auto base = loadOf(f.db, 1);

    SUBCASE("theme — только light/dark/system")
    {
        SettingsRecord out;
        Json::Value body;
        body["theme"] = "pink";
        const auto r = patchOf(base, body, out);
        CHECK_FALSE(r.ok);
        CHECK(r.errorCode == "bad_request");
        // base не испорчен отказом
        CHECK(out.theme == base.theme);
    }
    SUBCASE("lang — только ru/en/zh (5.18), дефолт не тронут отказом")
    {
        SettingsRecord out;
        Json::Value body;
        body["lang"] = "de";  // живой смок шага 2: PUT lang=de → 400
        const auto r = patchOf(base, body, out);
        CHECK_FALSE(r.ok);
        CHECK(r.errorCode == "bad_request");
        CHECK(out.lang == base.lang);  // base.lang == "ru" — не испорчен
        // каждый допустимый код принимается; недопустимые формы — нет
        for (const char* ok : {"ru", "en", "zh"})
        {
            Json::Value b;
            b["lang"] = ok;
            SettingsRecord o;
            REQUIRE(patchOf(base, b, o).ok);
            CHECK(o.lang == ok);
        }
        for (const char* bad : {"de", "RU", "zh-CN", "", "ru "})
        {
            Json::Value b;
            b["lang"] = bad;
            SettingsRecord o;
            const auto r = patchOf(base, b, o);
            // шаг 7: все невалидные формы — единый 400-контракт 02 §5.1
            // (errorCode «bad_request», а не просто отказ без кода)
            CHECK_FALSE(r.ok);
            CHECK(r.errorCode == "bad_request");
        }
        // не-строка (число) — тот же 400-контракт bad_request
        Json::Value num;
        num["lang"] = 1;
        SettingsRecord o2;
        const auto rn = patchOf(base, num, o2);
        CHECK_FALSE(rn.ok);
        CHECK(rn.errorCode == "bad_request");
    }
    SUBCASE("невалидный JSON в quiet_hours_json / onboarding_json → ошибка")
    {
        SettingsRecord out;
        Json::Value body;
        body["quiet_hours_json"] = "{не-json";
        CHECK_FALSE(patchOf(base, body, out).ok);
        Json::Value body2;
        body2["onboarding_json"] = "нет";
        CHECK_FALSE(patchOf(base, body2, out).ok);
    }
    SUBCASE("частичное обновление: отсутствующие поля не меняются")
    {
        SettingsRecord out;
        Json::Value body;
        body["theme"] = "dark";
        const auto r = patchOf(base, body, out);
        REQUIRE(r.ok);
        CHECK(out.theme == "dark");
        CHECK(out.notifySchedule == base.notifySchedule);
        CHECK(out.quietHoursJson == base.quietHoursJson);
        CHECK(out.onboardingJson == base.onboardingJson);
        CHECK(out.lang == base.lang);  // lang без значения в теле не меняется
    }
    SUBCASE("user_id из тела игнорируется — userId приходит из сессии")
    {
        SettingsRecord out;
        Json::Value body;
        body["user_id"] = 999;      // попытка подмены
        body["theme"] = "light";
        const auto r = patchOf(base, body, out);
        REQUIRE(r.ok);
        CHECK(out.userId == base.userId);  // остался от сессии, не 999
    }
}
