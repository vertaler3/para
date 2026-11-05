// Юнит-тесты OwnershipGuard (Э2 шаг 1; 02 §6.3, гайд BE §4 Э2):
//  - своё → Own (продолжаем);
//  - чужое (существует, другой user_id) → Forbidden → 403;
//  - несуществующее → NotFound → 404;
//  - ошибка БД → Error (fail-closed: не Own).
// Guard проверяется через настоящий DbClient на временной SQLite —
// как тесты Э1 на временных файлах (migration_runner_test.cpp): схема
// минимальная (id + user_id — ровно то, что читает guard), фикстуры
// вносятся напрямую sqlite3 API, каждый вызов guard'а синхронизируется
// promise/future (callback'и DrogonDbClient — с внутреннего потока).
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/OwnershipGuard.h"

namespace fs = std::filesystem;
using para::OwnershipGuard;
using Verdict = OwnershipGuard::Verdict;
using Table = OwnershipGuard::Table;

namespace {

// Временный каталог с уникальным именем; удаляется в конце теста.
struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-og-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

// Создание схемы/фикстур напрямую sqlite3 API — до открытия DbClient.
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

struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    Fixture(bool withTables = true)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        if (withTables)
        {
            // id/user_id — единственные столбцы, которые читает guard
            // (реальные таблицы: 0002/0003/0004).
            exec(raw,
                "CREATE TABLE applications(id INTEGER PRIMARY KEY,"
                " user_id INTEGER NOT NULL);"
                "CREATE TABLE grades(id INTEGER PRIMARY KEY,"
                " user_id INTEGER NOT NULL);"
                "CREATE TABLE notifications(id INTEGER PRIMARY KEY,"
                " user_id INTEGER NOT NULL);"
                "INSERT INTO applications(id, user_id) VALUES (1, 101), (2, 202);"
                "INSERT INTO grades(id, user_id) VALUES (10, 101);"
                "INSERT INTO notifications(id, user_id) VALUES (20, 202);");
        }
        sqlite3_close(raw);
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }
};

// Один вызов guard'а с ожиданием вердикта (cb Drogon вызывает с
// внутреннего потока — синхронизируем future'ом).
Verdict verdictOf(const drogon::orm::DbClientPtr& db,
                  Table table,
                  int64_t sessionUserId,
                  int64_t objectId)
{
    std::promise<Verdict> promise;
    auto future = promise.get_future();
    OwnershipGuard::check(db,
                          table,
                          sessionUserId,
                          objectId,
                          [&promise](Verdict v) { promise.set_value(v); });
    REQUIRE(future.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return future.get();
}

}  // namespace

TEST_CASE("OwnershipGuard: своё → Own (все три таблицы белого списка)")
{
    Fixture f;
    CHECK(verdictOf(f.db, Table::Applications, 101, 1) == Verdict::Own);
    CHECK(verdictOf(f.db, Table::Grades, 101, 10) == Verdict::Own);
    CHECK(verdictOf(f.db, Table::Notifications, 202, 20) == Verdict::Own);
}

TEST_CASE("OwnershipGuard: чужое (существует, другой владелец) → Forbidden → 403")
{
    Fixture f;
    // Аня (101) подставляет id чужой заявки (владелец 202) — 02 §6.3.
    CHECK(verdictOf(f.db, Table::Applications, 101, 2) == Verdict::Forbidden);
    // и в другую сторону: 202 открывает оценку 101.
    CHECK(verdictOf(f.db, Table::Grades, 202, 10) == Verdict::Forbidden);
    // уведомление 202 при сессии 101
    CHECK(verdictOf(f.db, Table::Notifications, 101, 20) == Verdict::Forbidden);
}

TEST_CASE("OwnershipGuard: несуществующее → NotFound → 404 (не маскируется под Forbidden)")
{
    Fixture f;
    CHECK(verdictOf(f.db, Table::Applications, 101, 999) == Verdict::NotFound);
    CHECK(verdictOf(f.db, Table::Notifications, 202, 999) == Verdict::NotFound);
}

TEST_CASE("OwnershipGuard: ошибка БД → Error, fail-closed (не Own)")
{
    // Вторая БД без таблиц: запрос падает DrogonDbException — вердикт
    // обязан быть Error (контроллер отвечает 500), никак не Own.
    Fixture f(/*withTables=*/false);
    CHECK(verdictOf(f.db, Table::Applications, 101, 1) == Verdict::Error);
}
