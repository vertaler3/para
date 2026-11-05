// security_tests — автотест безопасности 02 §9.3 (DoD №2: «проверка
// выполняется фактически, не декларируется»; вывод прикладывается к отчёту).
//
// Статус по шагам 02 §9.3 (честно — имитация успеха запрещена, AGENTS.md):
//   шаг 4 (grades/{id}: чужой → 403, свой → 200, несуществующий → 404) —
//     РЕАЛИЗОВАН здесь (Э4 шаг 3) на уровне OwnershipGuard::check(Table::Grades)
//     + GradesService::gradeById поверх временной SQLite с оценками
//     sql/fixtures.sql — тот же код, что под HTTP-контроллером
//     GradesCtrl::getGrade (верdict guard'а → тот же маппинг 403/404/500);
//     полный HTTP-вариант с двумя ЖИВЫМИ сессиями (cookie) — Э8;
//   шаги 1–3, 5–8 (заявки/уведомления/staff/cookie/брони) — Э8: их сущности
//     и эндпоинты появляются на Э5–Э6а, до того проверка этих строк —
//     вручную (curl с двумя сессиями).
//
// Две сессии — постановка Э4 шаг 3 (в §9.3 базовой редакции пары Аня/Олег;
// оценки в фикстурах есть только у Марка): user 1 = Аня (успеваемость
// ПУСТАЯ — 0 строк grades, средний null), user 3 = Марк (10 оценок, ids
// 1–10, включая чужой для Ани id 1). «Сессия» на этом уровне — user_id,
// который контроллер берёт из атрибута LoginFilter (02 §6.3).
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <drogon/orm/DbClient.h>

#include "services/GradesService.h"
#include "services/OwnershipGuard.h"

namespace fs = std::filesystem;
using para::GradesService;
using para::OwnershipGuard;

namespace {

int g_checks = 0;
int g_failed = 0;

void check(bool ok, const std::string& what)
{
    ++g_checks;
    if (ok)
    {
        std::printf("  PASS  %s\n", what.c_str());
        return;
    }
    ++g_failed;
    std::printf("  FAIL  %s\n", what.c_str());
}

// Временная SQLite — как в tests/unit/grades_service_test.cpp (Э4 шаги 1–2):
// схема disciplines/grades без FK + строки оценок sql/fixtures.sql дословно
// (все 10 — user_id=3); у Ани (user 1) оценок нет, session_plan не нужен
// (шаг 4 §9.3 — про grades/{id}).
struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        path = fs::temp_directory_path() /
               ("para-security-" + std::to_string(stamp));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void exec(sqlite3* db, const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK)
    {
        std::printf("FAIL  schema: %s\n", err ? err : "?");
        if (err != nullptr)
            sqlite3_free(err);
        std::exit(1);  // схема теста — предусловие, продолжать нет смысла
    }
}

void createSchema(const std::string& dbPath)
{
    sqlite3* raw = nullptr;
    if (sqlite3_open_v2(dbPath.c_str(),
                        &raw,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                        nullptr) != SQLITE_OK)
    {
        std::printf("FAIL  sqlite3_open(%s)\n", dbPath.c_str());
        std::exit(1);
    }
    exec(raw,
         "CREATE TABLE disciplines (id INTEGER PRIMARY KEY, code TEXT NOT"
         " NULL, title TEXT NOT NULL, control_type TEXT NOT NULL);"
         "INSERT INTO disciplines VALUES"
         " (1,'ME-101','Мировая экономика','exam'),"
         " (3,'HME-101','История МЭО','exam'),"
         " (6,'MT-401','Международная торговля','graded_credit'),"
         " (7,'EC-401','Эконометрика','exam'),"
         " (8,'BE-401','Деловой английский','credit'),"
         " (9,'MN-401','Международные переговоры','exam');"
         "CREATE TABLE grades ("
         "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  user_id INTEGER NOT NULL,"
         "  discipline_id INTEGER NOT NULL,"
         "  control_type TEXT NOT NULL,"
         "  component TEXT NOT NULL,"
         "  score INTEGER NOT NULL,"
         "  max_score INTEGER NOT NULL,"
         "  attempt INTEGER NOT NULL DEFAULT 1,"
         "  comment TEXT,"
         "  graded_at TEXT NOT NULL);"
         "CREATE INDEX idx_grades_user ON grades(user_id, discipline_id);"
         // Оценки sql/fixtures.sql дословно — все user_id=3 (Марк).
         "INSERT INTO grades(id, user_id, discipline_id, control_type,"
         " component, score, max_score, attempt, comment, graded_at)"
         " VALUES"
         " (1,  3, 1, 'exam',          'модуль 1',  30, 30, 1, NULL,"
         "  '2026-10-05T12:00:00Z'),"
         " (2,  3, 1, 'exam',          'модуль 2',  30, 30, 1, NULL,"
         "  '2026-10-19T12:00:00Z'),"
         " (3,  3, 1, 'exam',          'семинары',  18, 22, 1, NULL,"
         "  '2026-10-31T12:00:00Z'),"
         " (4,  3, 6, 'graded_credit', 'модуль 1',  27, 30, 1, NULL,"
         "  '2026-10-08T12:00:00Z'),"
         " (5,  3, 6, 'graded_credit', 'реферат',   28, 30, 1, NULL,"
         "  '2026-10-22T12:00:00Z'),"
         " (6,  3, 7, 'exam',          'модуль 1',  24, 30, 1, NULL,"
         "  '2026-10-07T12:00:00Z'),"
         " (7,  3, 7, 'exam',          'модуль 2',  26, 30, 1, NULL,"
         "  '2026-10-21T12:00:00Z'),"
         " (8,  3, 8, 'credit',        'зачёт',     60, 60, 1, NULL,"
         "  '2026-10-14T12:00:00Z'),"
         " (9,  3, 9, 'exam',          'модуль 1',  25, 30, 1, NULL,"
         "  '2026-10-09T12:00:00Z'),"
         " (10, 3, 3, 'exam',          'экзамен',   25, 60, 1,"
         "  'Порог не достигнут; пересдача 18 ноября',"
         "  '2026-06-24T12:00:00Z');");
    sqlite3_close(raw);
}

OwnershipGuard::Verdict guardGrade(const drogon::orm::DbClientPtr& db,
                                   int64_t sessionUserId,
                                   int64_t gradeId)
{
    // Ровно тот вызов, что делает GradesCtrl::getGrade до любых данных
    // (правило код-ревью OwnershipGuard.h); вердикты маппятся в HTTP:
    // Own → 200, NotFound → 404, Forbidden → 403, Error → 500.
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db,
                          OwnershipGuard::Table::Grades,
                          sessionUserId,
                          gradeId,
                          [&p](OwnershipGuard::Verdict v) { p.set_value(v); });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return OwnershipGuard::Verdict::Error;
    return f.get();
}

GradesService::OneResult gradeByIdOf(const drogon::orm::DbClientPtr& db,
                                     int64_t gradeId)
{
    std::promise<GradesService::OneResult> p;
    auto f = p.get_future();
    GradesService::gradeById(db,
                             gradeId,
                             [&p](GradesService::OneResult r) {
                                 p.set_value(std::move(r));
                             });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::OneResult{GradesService::One::Error, {}};
    return f.get();
}

GradesService::Result listOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::list(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::Result{false, {}};
    return f.get();
}

GradesService::Result summaryOf(const drogon::orm::DbClientPtr& db,
                                int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::summary(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::Result{false, {}};
    return f.get();
}

}  // namespace

int main()
{
    std::printf("security_tests — автотест безопасности 02 §9.3 (DoD №2).\n");
    std::printf(
        "Статус: шаг 4 (grades/{id}) — автопроверка; шаги 1–3, 5–8 — Э8.\n\n");

    // --- Сессия 1: Аня (user 1) — пустая успеваемость (состояние фикстур).
    // --- Сессия 2: Марк (user 3) — 10 оценок (ids 1–10).
    TempDir tmp;
    const std::string dbPath = (tmp.path / "para.db").string();
    createSchema(dbPath);
    auto db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);

    std::printf("Шаг 4 (02 §9.3): grades/{id} — две сессии (Аня user 1 / "
                "Марк user 3).\n");

    // Предусловие сценария: успеваемость Ани пуста (иначе «чужой grade»
    // не определён), у Марка — ровно 10 оценок фикстур.
    const auto anyaList = listOf(db, 1);
    check(anyaList.ok && anyaList.body["grades"].size() == 0,
          "предусловие: список оценок Ани пуст (пустая успеваемость)");
    const auto anyaSummary = summaryOf(db, 1);
    check(anyaSummary.ok && anyaSummary.body["average"].isNull(),
          "предусловие: сводка Ани — average=null (пустое состояние)");
    const auto markList = listOf(db, 3);
    check(markList.ok && markList.body["grades"].size() == 10,
          "предусловие: у Марка 10 оценок (ids 1–10, фикстуры)");

    // Чужой grade → Forbidden (контроллер отвечает 403, формат 02 §5.1):
    // сессия Ани против КАЖДОЙ оценки Марка — не только id 1 из §9.3.
    bool foreignAllForbidden = true;
    for (int64_t id = 1; id <= 10; ++id)
    {
        if (guardGrade(db, 1, id) != OwnershipGuard::Verdict::Forbidden)
        {
            foreignAllForbidden = false;
            std::printf("        чужой grade id=%lld — не Forbidden\n",
                        static_cast<long long>(id));
        }
    }
    check(foreignAllForbidden,
          "чужой grade (Аня → ids 1..10 Марка) → Forbidden (403) — 10/10");

    // Свой grade → Own, затем данные (контроллер → 200 JSON).
    check(guardGrade(db, 3, 1) == OwnershipGuard::Verdict::Own,
          "свой grade (Марк → id 1) → Own");
    const auto own = gradeByIdOf(db, 1);
    check(own.status == GradesService::One::Found &&
              own.body["id"].as<int64_t>() == 1 &&
              own.body["score"].as<int64_t>() == 30,
          "свой grade (Марк → id 1) → 200/Ok (Found, score=30)");
    const auto ownRetake = gradeByIdOf(db, 10);
    check(ownRetake.status == GradesService::One::Found &&
              ownRetake.body["comment"].asString() ==
                  "Порог не достигнут; пересдача 18 ноября",
          "свой grade (Марк → id 10) → 200/Ok (неуд с комментарием)");

    // Несуществующий → NotFound (404), для ОБОИХ сессий — не маскируется
    // под Forbidden и не раскрывает чужие данные.
    check(guardGrade(db, 1, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующий grade (Аня → 999999) → NotFound (404)");
    check(guardGrade(db, 3, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующий grade (Марк → 999999) → NotFound (404)");

    // Fail-closed: ошибка БД — Error (контроллер → 500), не разрешение
    // (02 §6.3: чужое не отдаётся никогда).
    {
        TempDir broken;
        auto brokenDb = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + (broken.path / "para.db").string(), 1);
        check(guardGrade(brokenDb, 3, 1) == OwnershipGuard::Verdict::Error,
              "ошибка БД → Error (500, fail-closed — не Own)");
    }

    std::printf("\nИтог шага 4: %d/%d проверок пройдено.\n",
                g_checks - g_failed, g_checks);
    if (g_failed != 0)
    {
        std::printf("security_tests: ПРОВАЛ — найдены нарушения изоляции "
                    "grades/{id}.\n");
        return 1;
    }
    std::printf("security_tests: шаг 4 ПРОЙДЕН (чужой → 403, свой → 200, "
                "несуществующий → 404).\n");
    std::printf("security_tests: шаги 1–3, 5–8 автотеста 02 §9.3 — этап Э8.\n");
    return 0;
}
