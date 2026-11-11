// Юнит-тесты аутентификации (шаг 7 Э1; 02 §9.1: PasswordHasher round-trip,
// лимитер входа — нарастающая задержка и сброс после успеха).
// Демо-пароль здесь НЕ используется: PasswordHasher тестируется на
// собственном тестовом пароле (демо-пароль в репо не попадает, AGENTS §5.2).
#include <doctest.hpp>

#include <string>

#include "services/PasswordHasher.h"
#include "utils/LoginLimiter.h"

using para::LoginLimiter;
using para::PasswordHasher;

TEST_CASE("PasswordHasher: round-trip argon2id, параметры OWASP в encoded")
{
    const std::string hash = PasswordHasher::hash("unit-test-password-2026");
    // encoded-строка PHC с параметрами 02 §6.2: t=2, m=19456, p=1
    CHECK(hash.rfind("$argon2id$v=19$m=19456,t=2,p=1$", 0) == 0);
    CHECK(PasswordHasher::verify(hash, "unit-test-password-2026"));
    CHECK_FALSE(PasswordHasher::verify(hash, "wrong-password"));
    // разные вызовы — разные соли (соль генерируется на каждый хэш)
    CHECK(PasswordHasher::hash("unit-test-password-2026") != hash);
}

TEST_CASE("PasswordHasher: verifyDummy — фиктивный прогон той же стоимости "
          "(Э8 шаг 4, timing-оракул Э1)")
{
    // Результат заведомо false при любом пароле (хэш паддинга), сам прогон
    // не бросает; равенство ВРЕМЕНИ с verify() не проверяем порогом —
    // хрупко в CI, факт замера обеих веток фиксирует журнал Э8 шаг 4
    // (реальный HTTP: 6.6 мс против 27.1 мс до правки → выровнено).
    CHECK_FALSE(PasswordHasher::verifyDummy("unit-test-password-2026"));
    CHECK_FALSE(PasswordHasher::verifyDummy(""));
}

TEST_CASE("LoginLimiter: 3 неудачи → блок 15 с, удвоение до потолка 15 мин")
{
    LoginLimiter limiter;
    const std::string login = "test.demo";
    int64_t t = 1'000'000;

    // до 3-й неудачи подряд вход разрешён (03 §4.1)
    limiter.onFailure(login, t);
    CHECK(limiter.blockedFor(login, t) == 0);
    limiter.onFailure(login, t + 1);
    CHECK(limiter.blockedFor(login, t + 1) == 0);
    // 3-я подряд — блок 15 с; внутри блока — оставшееся время
    limiter.onFailure(login, t + 2);
    CHECK(limiter.blockedFor(login, t + 2) == LoginLimiter::kInitialLockSec);
    CHECK(limiter.blockedFor(login, t + 2 + 14) == 1);
    // блок истёк в (t+2)+15 — с этой секунды вход снова разрешён
    t += 2 + LoginLimiter::kInitialLockSec;

    // после истечения блока каждая следующая неудача удваивает паузу:
    // 30 → 60 → 120 → 240 → 480 → 960 → потолок 900 с (15 минут)
    int64_t expect = LoginLimiter::kInitialLockSec * 2;
    while (true)
    {
        CHECK(limiter.blockedFor(login, t) == 0);
        limiter.onFailure(login, t);
        const int64_t capped =
            expect > LoginLimiter::kMaxLockSec ? LoginLimiter::kMaxLockSec
                                               : expect;
        CHECK(limiter.blockedFor(login, t) == capped);
        if (capped == LoginLimiter::kMaxLockSec)
            break;
        t += capped;
        expect *= 2;
    }
    // на потолке дальнейшие неудачи держат 900 с
    t += LoginLimiter::kMaxLockSec;
    CHECK(limiter.blockedFor(login, t) == 0);
    limiter.onFailure(login, t);
    CHECK(limiter.blockedFor(login, t) == LoginLimiter::kMaxLockSec);
}

TEST_CASE("LoginLimiter: успех сбрасывает счётчик; логины независимы")
{
    LoginLimiter limiter;
    const int64_t t = 5'000'000;
    limiter.onFailure("a.demo", t);
    limiter.onFailure("a.demo", t);
    limiter.onFailure("b.demo", t);
    CHECK(limiter.failures("a.demo") == 2);
    CHECK(limiter.failures("b.demo") == 1);
    CHECK(limiter.blockedFor("a.demo", t) == 0);
    CHECK(limiter.blockedFor("b.demo", t) == 0);
    // успех a.demo сбрасывает только a.demo
    limiter.onSuccess("a.demo");
    CHECK(limiter.failures("a.demo") == 0);
    CHECK(limiter.failures("b.demo") == 1);
}

// Э8 шаг 5 (урок Э2, латентные аналоги из находки Э2/Э5а): error-ветки
// цепочек UserService/SessionService обязаны получить ЖИВОЙ колбэк. До
// фикса `cb = std::move(cb)` в success-ветке оставлял error-ветке пустой
// std::function → std::bad_function_call (SIGABRT) на первом же сбое БД —
// тот же крах, что поймала Э2 шаг 1 в OwnershipGuard. БД БЕЗ таблиц →
// SELECT/DELETE падают DrogonDbException → колбэк обязан выстрелить ровно
// один раз с fail-значением.
#include <chrono>
#include <ctime>
#include <filesystem>
#include <future>
#include <thread>

#include <drogon/drogon.h>
#include <sqlite3.h>

#include "services/SessionService.h"
#include "services/UserService.h"

namespace {

// Временный каталог с уникальным именем; удаляется в конце теста
// (тот же приём, что в ownership_guard_test.cpp).
struct TempDir
{
    std::filesystem::path path;
    TempDir()
        : path(std::filesystem::temp_directory_path() /
               ("para-auth-unit-" +
                std::to_string(std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count())))
    {
        std::filesystem::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

}  // namespace

TEST_CASE("UserService/SessionService: ошибка БД → cb стреляет ровно один "
          "раз с fail-значением (Э8 шаг 5, double-move Э2)")
{
    TempDir tmp;
    // FK=ON — тот же initialization_queries, что в проде (Э8 шаг 5).
    auto db = drogon::orm::DbClient::newSqlite3Client(
        "filename=" + (tmp.path / "para.db").string(), 1,
        {"PRAGMA foreign_keys=ON;"});

    SUBCASE("UserService::findByLogin → cb(nullopt), не bad_function_call")
    {
        std::promise<bool> fired;
        para::UserService::findByLogin(db,
                                 "nobody.demo",
                                 [&fired](std::optional<para::UserRecord> r) {
                                     fired.set_value(!r.has_value());
                                 });
        CHECK(fired.get_future().get());
    }

    SUBCASE("SessionService::findValid → cb(nullopt)")
    {
        std::promise<bool> fired;
        para::SessionService::findValid(db,
                                  "token-no-such-table",
                                  [&fired](std::optional<para::SessionInfo> s) {
                                      fired.set_value(!s.has_value());
                                  });
        CHECK(fired.get_future().get());
    }

    SUBCASE("SessionService::create → cb(false, \"\")")
    {
        std::promise<bool> fired;
        para::SessionService::create(db,
                               1,
                               3600,
                               [&fired](bool ok, const std::string& token) {
                                   fired.set_value(!ok && token.empty());
                               });
        CHECK(fired.get_future().get());
    }

    SUBCASE("SessionService::remove → cb() выстрелил")
    {
        std::promise<bool> fired;
        para::SessionService::remove(db,
                               "token-no-such-table",
                               [&fired]() { fired.set_value(true); });
        CHECK(fired.get_future().get());
    }
}

// Э8 шаг 7 (находка Э1 «неполное зеркало лимитера»): доступ к действующей
// блокировке для зеркала users.locked_until — unix-сек, истёкший блок и
// неизвестный логин дают 0.
TEST_CASE("LoginLimiter: lockUntilSec — блок, истечение, неизвестный логин")
{
    LoginLimiter limiter;
    const std::string login = "mirror-lim.demo";
    CHECK(limiter.lockUntilSec(login, 1000) == 0);  // логина нет в карте
    for (int i = 0; i < 3; ++i)
        limiter.onFailure(login, 1000);
    CHECK(limiter.blockedFor(login, 1005) == 10);  // блок 15 с включён
    CHECK(limiter.lockUntilSec(login, 1005) == 1015);  // 1000 + kInitialLockSec
    CHECK(limiter.lockUntilSec(login, 1015) == 0);     // истёк (<= now)
    CHECK(limiter.lockUntilSec("other.demo", 1005) == 0);  // чужой логин
}

// Полное зеркало лимитера (02 §4.1/§6.4; Э8 шаг 7): неудача пишет
// failed_attempts И locked_until (ISO-UTC, 0 → NULL), успех сбрасывает оба;
// для несуществующего логина строки не создаёт (UPDATE-only). Зеркальный
// UPDATE асинхронный — значения читаются опросом с дедлайном (~2 с),
// busy_timeout на raw-соединении гасит коллизии с пулом.
TEST_CASE("UserService: зеркало лимитера — запись, ISO-формат, сброс, ghost-логин")
{
    TempDir tmp;
    const std::string dbPath = (tmp.path / "para.db").string();
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                            &raw,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                            nullptr) == SQLITE_OK);
    REQUIRE(sqlite3_exec(
                raw, "PRAGMA busy_timeout=2000;", nullptr, nullptr,
                nullptr) == SQLITE_OK);
    char* err = nullptr;
    REQUIRE(sqlite3_exec(
                raw,
                "CREATE TABLE users ("
                " id INTEGER PRIMARY KEY,"
                " login TEXT NOT NULL UNIQUE,"
                " role TEXT NOT NULL,"
                " password_hash TEXT NOT NULL,"
                " failed_attempts INTEGER NOT NULL DEFAULT 0,"
                " locked_until TEXT);"
                "INSERT INTO users (login, role, password_hash) VALUES"
                " ('mirror.demo','student','x');",
                nullptr, nullptr, &err) == SQLITE_OK);
    if (err != nullptr)
        sqlite3_free(err);

    auto db = drogon::orm::DbClient::newSqlite3Client(
        "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});

    const auto pollInt = [&](const char* sql, int64_t want) {
        for (int i = 0; i < 40; ++i)
        {
            sqlite3_stmt* st = nullptr;
            bool got = false;
            if (sqlite3_prepare_v2(raw, sql, -1, &st, nullptr) == SQLITE_OK &&
                sqlite3_step(st) == SQLITE_ROW)
            {
                got = sqlite3_column_int64(st, 0) == want;
            }
            sqlite3_finalize(st);
            if (got)
                return;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        FAIL("зеркало: опрос не дождался значения: ", sql);
    };
    const auto pollNull = [&](const char* sql, bool wantNull) {
        for (int i = 0; i < 40; ++i)
        {
            sqlite3_stmt* st = nullptr;
            bool got = false;
            if (sqlite3_prepare_v2(raw, sql, -1, &st, nullptr) == SQLITE_OK &&
                sqlite3_step(st) == SQLITE_ROW)
            {
                const bool isNull = sqlite3_column_type(st, 0) == SQLITE_NULL;
                got = isNull == wantNull;
            }
            sqlite3_finalize(st);
            if (got)
                return;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        FAIL("зеркало: опрос не дождался NULL-состояния: ", sql);
    };

    // 1) Неудача без блокировки (1 < 3, lockedUntilSec = 0) → счётчик 1,
    //    locked_until NULL.
    para::UserService::mirrorLimiterState(db, "mirror.demo", 1, 0);
    pollInt("SELECT failed_attempts FROM users WHERE login = 'mirror.demo'",
            1);
    pollNull("SELECT locked_until FROM users WHERE login = 'mirror.demo'",
             true);

    // 2) Неудача с блокировкой → locked_until записан в ISO-UTC «…Z»
    //    (тот же формат строк времени, что starts_at/ends_at).
    const int64_t lockSec = 1797096000;  // фиксированная метка, не wall-clock
    para::UserService::mirrorLimiterState(db, "mirror.demo", 3, lockSec);
    std::tm tm{};
    const std::time_t tt = static_cast<std::time_t>(lockSec);
    gmtime_r(&tt, &tm);
    char expected[24];
    std::strftime(expected, sizeof(expected), "%Y-%m-%dT%H:%M:%SZ", &tm);
    {
        std::string got;
        for (int i = 0; i < 40; ++i)
        {
            sqlite3_stmt* st = nullptr;
            if (sqlite3_prepare_v2(
                    raw,
                    "SELECT locked_until FROM users WHERE login = "
                    "'mirror.demo'",
                    -1, &st, nullptr) == SQLITE_OK &&
                sqlite3_step(st) == SQLITE_ROW &&
                sqlite3_column_type(st, 0) != SQLITE_NULL)
            {
                got = sqlite3_column_text(st, 0) != nullptr
                          ? reinterpret_cast<const char*>(
                                sqlite3_column_text(st, 0))
                          : "";
            }
            sqlite3_finalize(st);
            if (got.size() == 20 && got[10] == 'T' && got[19] == 'Z')
            {
                CHECK(got == expected);  // формат и значение
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if (i == 39)
                FAIL("зеркало: locked_until не записан в ISO-UTC «…Z»");
        }
    }

    // 3) Успешный вход → сброс обоих полей.
    para::UserService::resetLimiterMirror(db, "mirror.demo");
    pollInt("SELECT failed_attempts FROM users WHERE login = 'mirror.demo'",
            0);
    pollNull("SELECT locked_until FROM users WHERE login = 'mirror.demo'",
             true);

    // 4) Ghost-логин: строки в users НЕТ и не появляется (UPDATE-only —
    //    честное ограничение зеркала, UserService.h).
    para::UserService::mirrorLimiterState(db, "ghost.demo", 1, 0);
    pollInt("SELECT COUNT(*) FROM users WHERE login = 'ghost.demo'", 0);

    sqlite3_close(raw);
}
