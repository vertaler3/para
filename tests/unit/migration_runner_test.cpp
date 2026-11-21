// Юнит-тест миграционного раннера (гайд BE, Э1 шаг 4; 02 §8.3):
//  - идемпотентность: второй прогон не применяет ничего и не дублирует
//    фикстуры (маркер 'fixtures' — docs/decisions.md Р-5);
//  - дублёр счётчика PRAGMA user_version;
//  - ошибка в файле откатывает его транзакцию целиком (fail fast);
//  - --seed при отсутствующем fixtures.sql — не ошибка (файл появится на
//    шаге 6 Э1).
// Состояние БД проверяется напрямую через sqlite3 API — тест не доверяет
// раннеру проверять самого себя.
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/MigrationRunner.h"

namespace fs = std::filesystem;
using para::MigrationRunner;

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
               ("para-mr-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void writeFile(const fs::path& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary);
    file << content;
    REQUIRE(file.good());
}

// Скалярный запрос к файлу БД (count(*), PRAGMA).
std::string scalar(const std::string& dbPath, const std::string& sql)
{
    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                            &db,
                            SQLITE_OPEN_READONLY,
                            nullptr) == SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    REQUIRE(sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) ==
            SQLITE_OK);
    REQUIRE(sqlite3_step(stmt) == SQLITE_ROW);
    std::string out;
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    if (text != nullptr)
        out.assign(reinterpret_cast<const char*>(text));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return out;
}

}  // namespace

TEST_CASE("идемпотентность: второй прогон ничего не применяет и не дублирует фикстуры")
{
    TempDir tmp;
    const fs::path migrations = tmp.path / "migrations";
    fs::create_directories(migrations);
    writeFile(migrations / "0001_test_persons.sql",
              "CREATE TABLE persons(id INTEGER PRIMARY KEY, name TEXT NOT NULL);");
    writeFile(migrations / "0002_test_notes.sql",
              "CREATE TABLE notes(id INTEGER PRIMARY KEY, person_id INTEGER NOT NULL);");
    const fs::path fixtures = tmp.path / "fixtures.sql";
    writeFile(fixtures, "INSERT INTO persons(id, name) VALUES(1, 'Аня демо');");
    const std::string db = (tmp.path / "para.db").string();
    const std::string mig = migrations.string();
    const std::string fix = fixtures.string();

    SUBCASE("первый прогон с --seed применяет миграции и фикстуры")
    {
        const auto r1 = MigrationRunner::run(db, mig, fix, true);
        CHECK(r1.appliedMigrations == 2);
        CHECK(r1.fixturesApplied);
        CHECK(!r1.fixturesSkipped);
        CHECK(!r1.fixturesStale);
        // 2 миграции + 'fixtures' + 'fixtures-digest-…' (детекция 21.11)
        CHECK(scalar(db, "SELECT count(*) FROM schema_migrations") == "4");
        CHECK(scalar(db,
                     "SELECT count(*) FROM schema_migrations WHERE version "
                     "LIKE 'fixtures-digest-%'") == "1");
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
        // дублёр счётчика — номер последней миграции (02 §8.3)
        CHECK(scalar(db, "PRAGMA user_version") == "2");
        // WAL-режим (02 §7.5)
        CHECK(scalar(db, "PRAGMA journal_mode") == "wal");
    }

    SUBCASE("второй прогон с --seed ничего не меняет")
    {
        MigrationRunner::run(db, mig, fix, true);
        const auto r2 = MigrationRunner::run(db, mig, fix, true);
        CHECK(r2.appliedMigrations == 0);
        CHECK(!r2.fixturesApplied);
        CHECK(r2.fixturesSkipped);
        CHECK(!r2.fixturesStale);  // файл не менялся — сид актуален
        CHECK(scalar(db, "SELECT count(*) FROM schema_migrations") == "4");
        // фикстуры не задублировались — идемпотентность второго старта
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
        CHECK(scalar(db, "PRAGMA user_version") == "2");
    }

    SUBCASE("третий прогон уже без --seed: маркер фикстур остаётся, изменений нет")
    {
        MigrationRunner::run(db, mig, fix, true);
        const auto r3 = MigrationRunner::run(db, mig, fix, false);
        CHECK(r3.appliedMigrations == 0);
        CHECK(!r3.fixturesApplied);
        CHECK(!r3.fixturesSkipped);
        CHECK(scalar(db, "SELECT count(*) FROM schema_migrations") == "4");
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
    }
}

// Детекция устаревшего сида (дефект 21.11 «пустые Новости»): том засеян
// раньше, чем в fixtures.sql добавили данные, — маркер 'fixtures' стоит,
// идемпотентный skip молча не доносил новые строки. Теперь раннер сверяет
// отпечаток применённого файла с текущим и громко предупреждает, НЕ
// трогая данные (в демо-контуре могут быть живые изменения).
TEST_CASE("seed: изменившийся fixtures.sql → fixturesStale + WARN, данные не тронуты")
{
    TempDir tmp;
    const fs::path migrations = tmp.path / "migrations";
    fs::create_directories(migrations);
    writeFile(migrations / "0001_test_persons.sql",
              "CREATE TABLE persons(id INTEGER PRIMARY KEY, name TEXT NOT NULL);");
    const fs::path fixtures = tmp.path / "fixtures.sql";
    writeFile(fixtures, "INSERT INTO persons(id, name) VALUES(1, 'Аня демо');");
    const std::string db = (tmp.path / "para.db").string();
    const std::string mig = migrations.string();
    const std::string fix = fixtures.string();
    MigrationRunner::run(db, mig, fix, true);

    SUBCASE("файл изменился → stale при каждом seed-старте, данные прежние")
    {
        writeFile(fixtures,
                  "INSERT INTO persons(id, name) VALUES(1, 'Аня демо'),"
                  "(2, 'Марк демо');");
        const auto r = MigrationRunner::run(db, mig, fix, true);
        CHECK(r.fixturesSkipped);
        CHECK(r.fixturesStale);
        CHECK(!r.fixturesApplied);
        // новые строки НЕ залиты и старые НЕ тронуты — только предупреждение
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
        // повторный старт без изменений файла — по-прежнему stale (данные
        // в БД так и не совпадают с файлом)
        const auto r2 = MigrationRunner::run(db, mig, fix, true);
        CHECK(r2.fixturesStale);
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
    }

    SUBCASE("сид старее детекции (нет digest-строки) → stale")
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open(db.c_str(), &raw) == SQLITE_OK);
        char* err = nullptr;
        REQUIRE(sqlite3_exec(raw,
                             "DELETE FROM schema_migrations WHERE version "
                             "LIKE 'fixtures-digest-%';",
                             nullptr, nullptr, &err) == SQLITE_OK);
        sqlite3_close(raw);
        const auto r = MigrationRunner::run(db, mig, fix, true);
        CHECK(r.fixturesSkipped);
        CHECK(r.fixturesStale);
        CHECK(scalar(db, "SELECT count(*) FROM persons") == "1");
    }

    SUBCASE("отпечаток устойчив: fileDigest детерминирован и различает файлы")
    {
        const std::string a = MigrationRunner::fileDigest("abc");
        CHECK(a.size() == 16);
        CHECK(a == MigrationRunner::fileDigest("abc"));
        CHECK(a != MigrationRunner::fileDigest("abd"));
        CHECK(MigrationRunner::fileDigest("") != a);
    }
}

TEST_CASE("ошибка в файле откатывает его транзакцию целиком (fail fast)")
{
    TempDir tmp;
    const fs::path migrations = tmp.path / "migrations";
    fs::create_directories(migrations);
    writeFile(migrations / "0001_test_persons.sql",
              "CREATE TABLE persons(id INTEGER PRIMARY KEY, name TEXT NOT NULL);");
    // Валидный DDL + заведомо падающий INSERT: без отката таблица boom
    // осталась бы «полуприменённой».
    writeFile(migrations / "0002_test_bad.sql",
              "CREATE TABLE boom(id INTEGER PRIMARY KEY);\n"
              "INSERT INTO no_such_table VALUES(1);");
    const std::string db = (tmp.path / "para.db").string();
    const std::string mig = migrations.string();
    const std::string fix = (tmp.path / "fixtures.sql").string();

    bool threw = false;
    std::string message;
    try
    {
        MigrationRunner::run(db, mig, fix, false);
    }
    catch (const para::MigrationError& e)
    {
        threw = true;
        message = e.what();
    }
    REQUIRE(threw);
    // внятный лог: имя файла-виновника в сообщении
    CHECK(message.find("0002_test_bad") != std::string::npos);
    // 0001 применена и учтена, 0002 — нет
    CHECK(scalar(db, "SELECT count(*) FROM schema_migrations") == "1");
    CHECK(scalar(db, "SELECT count(*) FROM schema_migrations WHERE version='0001_test_persons'") == "1");
    // частично выполненный DDL файла откатился вместе с транзакцией
    CHECK(scalar(db,
                 "SELECT count(*) FROM sqlite_master WHERE type='table' AND "
                 "name='boom'") == "0");
    CHECK(scalar(db, "PRAGMA user_version") == "1");
}

TEST_CASE("--seed без файла фикстур — не ошибка (файл появится на шаге 6 Э1)")
{
    TempDir tmp;
    const fs::path migrations = tmp.path / "migrations";
    fs::create_directories(migrations);
    writeFile(migrations / "0001_test_persons.sql",
              "CREATE TABLE persons(id INTEGER PRIMARY KEY, name TEXT NOT NULL);");
    const std::string db = (tmp.path / "para.db").string();
    const std::string mig = migrations.string();
    const std::string absent = (tmp.path / "no_such_fixtures.sql").string();

    const auto r = MigrationRunner::run(db, mig, absent, true);
    CHECK(r.appliedMigrations == 1);
    CHECK(!r.fixturesApplied);
    CHECK(!r.fixturesSkipped);
    CHECK(scalar(db, "SELECT count(*) FROM schema_migrations") == "1");
}

TEST_CASE("collectMigrations: сортировка по номеру, не-SQL и мусор игнорируются")
{
    TempDir tmp;
    const fs::path migrations = tmp.path / "migrations";
    fs::create_directories(migrations);
    writeFile(migrations / "0010_late.sql", "-- поздняя");
    writeFile(migrations / "0002_two.sql", "-- вторая");
    writeFile(migrations / "0001_one.sql", "-- первая");
    writeFile(migrations / ".gitkeep", "");
    writeFile(migrations / "README.md", "не миграция");
    writeFile(migrations / "broken.sql", "нет числового префикса");

    const auto versions = MigrationRunner::collectMigrations(migrations.string());
    REQUIRE(versions.size() == 3);
    CHECK(versions[0] == "0001_one");
    CHECK(versions[1] == "0002_two");
    CHECK(versions[2] == "0010_late");
}
