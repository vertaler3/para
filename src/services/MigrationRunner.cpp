#include "services/MigrationRunner.h"

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>
#include <vector>

#include <sqlite3.h>
#include <trantor/utils/Logger.h>

namespace para {
namespace fs = std::filesystem;

namespace {

// Соединение sqlite3 с закрытием в деструкторе (RAII): раннер синхронный,
// одно соединение до старта event loop Drogon (docs/decisions.md Р-5).
class Db
{
  public:
    explicit Db(const std::string& path)
    {
        const int rc = sqlite3_open_v2(path.c_str(),
                                       &handle_,
                                       SQLITE_OPEN_READWRITE |
                                           SQLITE_OPEN_CREATE,
                                       nullptr);
        if (rc != SQLITE_OK)
        {
            const std::string msg =
                handle_ ? sqlite3_errmsg(handle_) : "sqlite3_open_v2 failed";
            close();
            throw MigrationError("не удалось открыть БД '" + path +
                                 "': " + msg);
        }
        // Короткий таймаут на случай конкурентного писца (например, старый
        // процесс ещё не закрыл БД при рестарте контейнера).
        sqlite3_busy_timeout(handle_, 5000);
    }

    ~Db()
    {
        close();
    }

    Db(const Db&) = delete;
    Db& operator=(const Db&) = delete;

    sqlite3* get() const
    {
        return handle_;
    }

  private:
    void close()
    {
        if (handle_ != nullptr)
        {
            sqlite3_close(handle_);
            handle_ = nullptr;
        }
    }

    sqlite3* handle_ = nullptr;
};

// Выполнить SQL (может быть несколько стейтментов, как в файле миграции).
void exec(sqlite3* db, const std::string& sql, const std::string& where)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK)
    {
        const std::string msg = err != nullptr ? err : sqlite3_errmsg(db);
        sqlite3_free(err);
        throw MigrationError(where + ": " + msg);
    }
}

// Скалярный запрос → строка (для PRAGMA и count(*)).
std::string scalar(sqlite3* db, const std::string& sql, const std::string& where)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        const std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw MigrationError(where + ": " + msg);
    }
    std::string out;
    if (sqlite3_step(stmt) == SQLITE_ROW &&
        sqlite3_column_type(stmt, 0) != SQLITE_NULL)
    {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text != nullptr)
            out.assign(reinterpret_cast<const char*>(text));
    }
    sqlite3_finalize(stmt);
    return out;
}

std::string readFile(const fs::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        throw MigrationError("не удалось прочитать '" + path.string() + "'");
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool numericPrefix(const std::string& name, long& number)
{
    // Паттерн файла миграции: NNNN_остальное.sql (0001_identity.sql, 02 §8.3).
    const size_t underscore = name.find('_');
    if (underscore == 0 || underscore == std::string::npos)
        return false;
    const auto result =
        std::from_chars(name.data(), name.data() + underscore, number);
    return result.ec == std::errc{} && result.ptr == name.data() + underscore;
}

// Применить один SQL-файл в транзакции вместе с записью версии.
// Файлы миграций пишутся БЕЗ своих BEGIN/COMMIT — границы ставит раннер.
// newUserVersion >= 0 — одновременно обновить дублёр PRAGMA user_version
// (той же транзакцией: откат файла откатывает и счётчик); -1 — не менять
// (фикстуры: это не схема).
void applyInTransaction(sqlite3* db,
                        const std::string& version,
                        const std::string& sql,
                        long newUserVersion)
{
    exec(db, "BEGIN IMMEDIATE;", "транзакция " + version);
    try
    {
        exec(db, sql, "миграция " + version);
        // Запись версии — параметром, не конкатенацией (AGENTS.md §5.5).
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db,
                               "INSERT INTO schema_migrations(version) "
                               "VALUES(?)",
                               -1,
                               &stmt,
                               nullptr) != SQLITE_OK)
        {
            const std::string msg = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw MigrationError("запись версии " + version + ": " + msg);
        }
        sqlite3_bind_text(stmt,
                          1,
                          version.c_str(),
                          static_cast<int>(version.size()),
                          SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            const std::string msg = sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            throw MigrationError("запись версии " + version + ": " + msg);
        }
        sqlite3_finalize(stmt);
        if (newUserVersion >= 0)
        {
            // Присваивание PRAGMA нельзя параметризовать — значение целое
            // из std::to_string(long), инъекция невозможна.
            exec(db,
                 "PRAGMA user_version = " + std::to_string(newUserVersion) +
                     ";",
                 "PRAGMA user_version " + version);
        }
        exec(db, "COMMIT;", "COMMIT " + version);
    }
    catch (...)
    {
        try
        {
            exec(db, "ROLLBACK;", "ROLLBACK " + version);
        }
        catch (const std::exception&)
        {
            // Не удалось откатить (соединение сломано) — исходная ошибка
            // важнее, БД остаётся без записи о неприменённой версии.
        }
        throw;
    }
}

std::set<std::string> appliedVersions(sqlite3* db)
{
    std::set<std::string> versions;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db,
                           "SELECT version FROM schema_migrations",
                           -1,
                           &stmt,
                           nullptr) != SQLITE_OK)
    {
        const std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw MigrationError("чтение schema_migrations: " + msg);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        if (text != nullptr)
            versions.insert(reinterpret_cast<const char*>(text));
    }
    sqlite3_finalize(stmt);
    return versions;
}

}  // namespace

std::vector<std::string> MigrationRunner::collectMigrations(
    const std::string& migrationsDir)
{
    std::vector<std::string> versions;
    std::error_code ec;
    if (!fs::is_directory(migrationsDir, ec))
    {
        throw MigrationError("каталог миграций не найден: '" + migrationsDir +
                             "'");
    }
    for (const auto& entry : fs::directory_iterator(migrationsDir, ec))
    {
        if (ec)
            break;
        if (!entry.is_regular_file())
            continue;
        const std::string filename = entry.path().filename().string();
        if (!filename.ends_with(".sql"))
            continue;  // .gitkeep и прочее — не миграции
        const std::string version = filename.substr(0, filename.size() - 4);
        long number = 0;
        if (!numericPrefix(version, number))
        {
            LOG_WARN << "миграционный раннер: файл без числового префикса "
                        "пропущен: " << filename;
            continue;
        }
        versions.push_back(version);
    }
    // По возрастанию номера: префикс NNNN фиксированной ширины, поэтому
    // лексикографическая сортировка имён == числовая (02 §8.3).
    std::sort(versions.begin(), versions.end());
    return versions;
}

MigrationRunner::Result MigrationRunner::run(const std::string& dbPath,
                                             const std::string& migrationsDir,
                                             const std::string& fixturesPath,
                                             bool seedMode)
{
    // Каталог БД (в контейнере — /opt/para/data на томе para-data) создаём
    // при необходимости: первый старт без БД должен работать.
    const fs::path parent = fs::path(dbPath).parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        fs::create_directories(parent, ec);
        if (ec)
        {
            throw MigrationError("не удалось создать каталог БД '" +
                                 parent.string() + "': " + ec.message());
        }
    }

    Result result;
    Db db(dbPath);
    sqlite3* handle = db.get();

    // WAL-режим + synchronous=NORMAL (02 §7.5). WAL персистентен в файле БД,
    // а соединения DbClient дополнительно ставят те же PRAGMA через
    // initialization_queries в config.json (docs/decisions.md Р-5).
    exec(handle, "PRAGMA journal_mode=WAL;", "PRAGMA journal_mode " + dbPath);
    const std::string mode =
        scalar(handle, "PRAGMA journal_mode;", "PRAGMA journal_mode " + dbPath);
    if (mode != "wal")
    {
        // WAL недоступен только на экзотических ФС (нет shared memory);
        // приложение валидно и в rollback-journal — предупреждаем, не падаем.
        LOG_WARN << "миграционный раннер: journal_mode=" << mode
                 << " вместо wal (БД '" << dbPath
                 << "'); продолжаем — см. docs/decisions.md Р-5";
    }
    exec(handle, "PRAGMA synchronous=NORMAL;", "PRAGMA synchronous");

    // Таблица учёта создаётся самим раннером (02 §8.3, гайд BE шаг 4);
    // applied_at — ISO 8601 UTC, как все времена в проекте (02 §4).
    exec(handle,
         "CREATE TABLE IF NOT EXISTS schema_migrations("
         "version TEXT PRIMARY KEY,"
         "applied_at TEXT NOT NULL DEFAULT "
         "(strftime('%Y-%m-%dT%H:%M:%SZ','now')));",
         "создание schema_migrations");

    std::set<std::string> applied = appliedVersions(handle);

    for (const std::string& version : collectMigrations(migrationsDir))
    {
        if (applied.count(version) != 0)
            continue;
        const std::string sql = readFile(fs::path(migrationsDir) /
                                         (version + ".sql"));
        // Дублёр PRAGMA user_version (02 §8.3) обновляется той же
        // транзакцией, что и запись версии, — рассинхрона с
        // schema_migrations не бывает и при откате упавшего файла.
        long number = 0;
        if (!numericPrefix(version, number))
            throw MigrationError("версия без числового префикса: " + version);
        applyInTransaction(handle, version, sql, number);
        applied.insert(version);
        ++result.appliedMigrations;
        LOG_INFO << "миграция применена: " << version;
    }

    if (seedMode)
    {
        if (!fs::exists(fixturesPath))
        {
            // Не ошибка: фикстуры появляются на шаге 6 Э1 — раннер обязан
            // уметь применить их, когда файл возникнет (задание шага 4).
            LOG_WARN << "--seed: файл фикстур не найден ('" << fixturesPath
                     << "'), сидирование пропущено";
        }
        else if (applied.count(kFixturesVersion) != 0)
        {
            LOG_INFO << "--seed: фикстуры уже применены (маркер '"
                     << kFixturesVersion
                     << "' в schema_migrations), повторное сидирование "
                        "пропущено";
            result.fixturesSkipped = true;
        }
        else
        {
            // Та же транзакционная схема, что и у миграций: содержимое файла
            // + маркер — атомарно; идемпотентность второго старта — маркер
            // (docs/decisions.md Р-5). user_version не трогаем: фикстуры —
            // не схема.
            applyInTransaction(handle,
                               kFixturesVersion,
                               readFile(fixturesPath),
                               -1);
            result.fixturesApplied = true;
            LOG_INFO << "фикстуры применены: " << fixturesPath;
        }
    }

    // Компенсирующий контроль Р-24 (Э8 шаг 5, находка Э1): enforcement FK
    // на соединении раннера НЕ включается — PRAGMA foreign_keys является
    // no-op внутри его транзакций (applyInTransaction; см. также Р-13),
    // schema-фаза по руководству SQLite выполняется с выключенным FK
    // (пересоздания таблиц/DROP с сылками), а порядок вставок в
    // fixtures.sql не обязан быть FK-упорядоченным. Вместо этого
    // согласованность данных валидируется ГАТЕМ на каждом старте:
    // foreign_key_check работает и при выключенном enforcement и
    // перечисляет ВСЕ висячие ссылки; нарушение — фатально (MigrationError),
    // т.е. несогласованную БД приложение не поднимет. Рантайм-enforcement
    // включён на соединениях пула приложения (config.json
    // initialization_queries, «PRAGMA foreign_keys=ON»).
    {
        sqlite3_stmt* fkStmt = nullptr;
        if (sqlite3_prepare_v2(handle,
                               "PRAGMA foreign_key_check;",
                               -1,
                               &fkStmt,
                               nullptr) != SQLITE_OK)
        {
            const std::string msg = sqlite3_errmsg(handle);
            sqlite3_finalize(fkStmt);
            throw MigrationError("PRAGMA foreign_key_check (" + dbPath +
                                 "): " + msg);
        }
        bool violated = false;
        int rc = SQLITE_OK;
        while ((rc = sqlite3_step(fkStmt)) == SQLITE_ROW)
        {
            const unsigned char* table = sqlite3_column_text(fkStmt, 0);
            const sqlite3_int64 rowid = sqlite3_column_int64(fkStmt, 1);
            LOG_ERROR << "миграционный раннер: висячая ссылка: таблица '"
                      << (table != nullptr
                              ? reinterpret_cast<const char*>(table)
                              : "?")
                      << "', rowid " << rowid;
            violated = true;
            break;  // первой достаточно для отказа старта
        }
        sqlite3_finalize(fkStmt);
        if (violated || rc != SQLITE_DONE)
        {
            throw MigrationError(
                violated
                    ? "PRAGMA foreign_key_check: данные нарушают внешние "
                      "ключи (" + dbPath + ") — см. лог; старт прерван (Р-24)"
                    : "PRAGMA foreign_key_check (" + dbPath + "): " +
                          sqlite3_errmsg(handle));
        }
    }

    LOG_INFO << "миграционный раннер: применено файлов — "
             << result.appliedMigrations << ", user_version — "
             << scalar(handle, "PRAGMA user_version;", "PRAGMA user_version")
             << ", foreign_key_check — ok"
             << (seedMode
                     ? (result.fixturesApplied
                            ? ", фикстуры применены"
                            : (result.fixturesSkipped
                                   ? ", фикстуры уже были"
                                   : ", фикстуры отсутствуют"))
                     : "");
    return result;
}

}  // namespace para
