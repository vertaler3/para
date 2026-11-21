#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace para {

// Ошибка применения миграций/фикстур. Получивший её main обязан упасть
// (fail fast) с внятным логом — деградации «поднялись без схемы» нет
// (гайд BE, Э1 шаг 4; 02 §8.3).
class MigrationError : public std::runtime_error
{
  public:
    explicit MigrationError(const std::string& what)
        : std::runtime_error(what)
    {
    }
};

// Миграционный раннер (02 §8.3): применяет /sql/migrations/NNNN_*.sql при
// старте приложения, каждый файл — в транзакции; учёт — в schema_migrations,
// дублёр счётчика для SQLite — PRAGMA user_version. Таблицу schema_migrations
// создаёт сам. Режим --seed применяет sql/fixtures.sql идемпотентно — маркер
// 'fixtures' в schema_migrations (docs/decisions.md Р-5).
//
// Синхронный, на системной libsqlite3, выполняется в main до app().run():
// DbClient Drogon асинхронен и создаётся только в run(), а миграции обязаны
// пройти до поднятия роутов (docs/decisions.md Р-5).
class MigrationRunner
{
  public:
    struct Result
    {
        // Сколько файлов миграций применено в этом прогоне (0 — все уже были).
        int appliedMigrations = 0;
        // Фикстуры применены в этом прогоне (только при seedMode).
        bool fixturesApplied = false;
        // seedMode, но маркер 'fixtures' уже стоит — корректный пропуск.
        bool fixturesSkipped = false;
        // seedMode, фикстуры пропущены, НО fixtures.sql не совпадает с
        // применённым сидом (или сид старее детекции): в БД нет данных,
        // добавленных в фикстуры позже первого сида. Не ошибка (данные
        // демо-контура могут содержать живые изменения) — громкий WARN в
        // лог с инструкцией пересева (дефект «пустые Новости» 21.11:
        // том засеян до news-блока фикстур, сид молча пропускался).
        bool fixturesStale = false;
    };

    // dbPath — файл SQLite (каталог создаётся при необходимости);
    // migrationsDir — каталог с файлами NNNN_name.sql (по возрастанию номера);
    // fixturesPath — sql/fixtures.sql для seedMode (отсутствие файла — не
    // ошибка: фикстуры появляются на шаге 6 Э1);
    // seedMode — флаг --seed.
    // Бросает MigrationError при любой ошибке: БД остаётся в состоянии до
    // файла, который не смог примениться.
    static Result run(const std::string& dbPath,
                      const std::string& migrationsDir,
                      const std::string& fixturesPath,
                      bool seedMode);

    // Отсортированные по возрастанию номера версии (имя файла без .sql)
    // из migrationsDir; не-SQL и файлы без числового префикса игнорируются.
    // Публично для юнит-тестов.
    static std::vector<std::string> collectMigrations(
        const std::string& migrationsDir);

    // Маркер фикстур в schema_migrations (docs/decisions.md Р-5).
    static constexpr const char* kFixturesVersion = "fixtures";
    // Префикс строки-отпечатка применённого fixtures.sql (FNV-1a, hex):
    // 'fixtures-digest-<hex>'; записывается сразу после фикстур той же
    // схемой учёта. При seed-старте отпечаток сверен с текущим файлом —
    // расхождение/отсутствие = fixturesStale (WARN, пересев руками).
    static constexpr const char* kFixturesDigestPrefix = "fixtures-digest-";

    // Отпечаток содержимого файла (64-бит FNV-1a, 16 hex-символов). Не
    // криптография — детектор изменений fixtures.sql между сидами.
    // Публично для юнит-тестов.
    static std::string fileDigest(const std::string& content);
};

}  // namespace para
