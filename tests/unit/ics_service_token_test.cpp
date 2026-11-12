// Юнит-тесты контракта токена подписки ICS (Р-11 — правки по гейту Э3,
// раунды 1–2): длина 64 hex-символа (256 бит энтропии), hex-алфавит,
// ротация (UPSERT по user_id — строка одна, токен меняется), отзыв
// идемпотентен; токен СЕССИИ остаётся 128 бит / 32 hex (Р-7, контракт Э1).
// Временная SQLite: users + ics_tokens (миграция 0004, БЕЗ FK на users —
// FK тест не проверяет). IcsService и SessionService линкуются в para-tests.
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/IcsService.h"
#include "services/SessionService.h"

namespace fs = std::filesystem;
using para::IcsService;
using para::SessionService;

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
               ("para-ics-" + std::to_string(stamp) + "-" +
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

// users + ics_tokens — колонки миграции 0004 без FK.
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
             "CREATE TABLE users (id INTEGER PRIMARY KEY, login TEXT NOT"
             " NULL, role TEXT NOT NULL);"
             "INSERT INTO users VALUES (1,'anya.demo','student'),"
             " (3,'mark.demo','student');"
             "CREATE TABLE ics_tokens ("
             "  user_id    INTEGER PRIMARY KEY,"
             "  token      TEXT NOT NULL UNIQUE,"
             "  created_at TEXT NOT NULL,"
             "  revoked    INTEGER NOT NULL DEFAULT 0);");
        sqlite3_close(raw);
        // FK на каждом соединении (Э8 шаг 5, находка Э1) — те же
        // initialization_queries, что в проде (config.json); Drogon
        // 1.9.14 исполняет их на каждом новом соединении пула.
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }
};

std::string createOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<std::pair<bool, std::string>> p;
    auto f = p.get_future();
    IcsService::createToken(db,
                            userId,
                            [&p](bool ok, const std::string& token) {
                                p.set_value({ok, token});
                            });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    const auto [ok, token] = f.get();
    REQUIRE(ok);
    return token;
}

bool revokeOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<bool> p;
    auto f = p.get_future();
    IcsService::revokeToken(db, userId, [&p](bool ok) { p.set_value(ok); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

IcsService::ActiveToken activeOf(const drogon::orm::DbClientPtr& db,
                                 int64_t userId)
{
    std::promise<IcsService::ActiveToken> p;
    auto f = p.get_future();
    IcsService::activeToken(
        db, userId, [&p](IcsService::ActiveToken t) {
            p.set_value(std::move(t));
        });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

bool allHex(const std::string& s)
{
    for (const char c : s)
        if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f'))
            return false;
    return true;
}

int scalarOf(const std::string& dbPath, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) ==
            SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    const int v = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    sqlite3_close(raw);
    return v;
}

}  // namespace

TEST_CASE("IcsService: createToken — 64 hex-символа (256 бит, Р-11)")
{
    Fixture f;
    const std::string token = createOf(f.db, 1);
    CHECK(token.size() == 64);  // 256 бит → 64 hex (порог гейта «токенДлина»)
    CHECK(allHex(token));
    CHECK(IcsService::stampNow().size() == 16);  // YYYYmmddTHHMMSSZ
    // В БД ровно одна строка пользователя, активная.
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM ics_tokens") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT revoked FROM ics_tokens WHERE user_id = 1") == 0);
    CHECK(scalarOf(f.dbPath,
                   "SELECT length(token) FROM ics_tokens WHERE user_id = 1") ==
          64);
}

TEST_CASE("IcsService: ротация — новый токен ≠ прежнего, строка одна")
{
    Fixture f;
    const std::string first = createOf(f.db, 1);
    const std::string second = createOf(f.db, 1);
    CHECK(first != second);
    CHECK(second.size() == 64);
    CHECK(allHex(second));
    // PK user_id: строка по-прежнему одна — прежний токен заменён.
    CHECK(scalarOf(f.dbPath, "SELECT count(*) FROM ics_tokens") == 1);
    CHECK(scalarOf(f.dbPath,
                   "SELECT count(*) FROM ics_tokens WHERE token = '" +
                       first + "'") == 0);
    // Уникальность на серии: 24 токена разных пользователей — все различны.
    std::set<std::string> seen;
    for (int64_t id = 10; id < 34; ++id)
        seen.insert(createOf(f.db, id));
    CHECK(seen.size() == 24);
    for (const auto& t : seen)
    {
        CHECK(t.size() == 64);
        CHECK(allHex(t));
    }
}

TEST_CASE("IcsService: revokeToken — отзыв и идемпотентность")
{
    Fixture f;
    const std::string token = createOf(f.db, 1);
    CHECK(revokeOf(f.db, 1));
    CHECK(scalarOf(f.dbPath,
                   "SELECT revoked FROM ics_tokens WHERE user_id = 1") == 1);
    CHECK(revokeOf(f.db, 1));  // повторный отзыв — не ошибка (204 в контроллере)
    CHECK(revokeOf(f.db, 42));  // отзов без активного токена — не ошибка
    // Токен пользователя 3 (фикс-аналог Марка) не затронут.
    CHECK(createOf(f.db, 3).size() == 64);
    CHECK(scalarOf(f.dbPath,
                   "SELECT revoked FROM ics_tokens WHERE user_id = 3") == 0);
    (void)token;
}

TEST_CASE("IcsService: activeToken — статус подписки (8.ii, GET /v1/ics/token)")
{
    Fixture f;
    // Нет строки — not found (контроллер → 404 «нет подписки»).
    const auto none = activeOf(f.db, 1);
    CHECK(none.ok);
    CHECK_FALSE(none.found);
    // Активный токен — тот же, что отдаёт фид GET /ics/{token}: источник
    // один (строка с revoked = 0), created_at — ISO UTC (SessionService::
    // utcNow, 20 символов «YYYY-MM-DDTHH:MM:SSZ»).
    const std::string token = createOf(f.db, 1);
    const auto active = activeOf(f.db, 1);
    CHECK(active.ok);
    CHECK(active.found);
    CHECK(active.token == token);
    CHECK(active.token.size() == 64);
    CHECK(active.createdAt.size() == 20);
    CHECK(active.createdAt.back() == 'Z');
    // Ротация (POST) меняет источник — activeToken отдаёт уже новый.
    const std::string rotated = createOf(f.db, 1);
    const auto afterRotate = activeOf(f.db, 1);
    CHECK(afterRotate.found);
    CHECK(afterRotate.token == rotated);
    CHECK(afterRotate.token != token);
    CHECK(afterRotate.createdAt.size() == 20);
    // Отзыв (DELETE) → not found; чужая подписка не затронута.
    CHECK(revokeOf(f.db, 1));
    const auto revoked = activeOf(f.db, 1);
    CHECK(revoked.ok);
    CHECK_FALSE(revoked.found);
    const std::string other = createOf(f.db, 3);
    CHECK(activeOf(f.db, 3).token == other);
    CHECK(activeOf(f.db, 42).found == false);  // никогда не было строки
    (void)token;
}

TEST_CASE("SessionService: токен сессии — 128 бит / 32 hex (Р-7, не менялся)")
{
    // Контракт Э1: generateToken() по умолчанию — 16 байт; ICS-токен берёт
    // явно 32 байта (Р-11). Параметризация не должна была сместить дефолт.
    const std::string session = SessionService::generateToken();
    CHECK(session.size() == 32);
    CHECK(allHex(session));
    const std::string ics = SessionService::generateToken(32);
    CHECK(ics.size() == 64);
    CHECK(ics != session);
}
