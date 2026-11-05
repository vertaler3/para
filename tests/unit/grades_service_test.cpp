// Юнит-тесты GradesService (Э4 шаги 1–2; 02 §5.2, миграция 0002):
//  - список — только СВОИ оценки (user_id из сессии), с кодом/названием
//    дисциплины, comment null у строк без комментария;
//  - сводка — агрегат по дисциплинам: порог = Σ max_score компонентов,
//    «что осталось» (missing) = max(0, Σ max_score − Σ score), средний балл —
//    взвешенный процент Σ score / Σ max_score × 100 (демо-арифметика
//    05 §3 Э4: у «Мировой экономики» 30+30+18=78 при пороге 82 — не хватает 4);
//  - одна запись (шаг 2): gradeById по PK — БЕЗ user_id-условия: владение
//    проверяет OwnershipGuard (сценарии своё/чужое/несуществующее/ошибка БД —
//    через OwnershipGuard::check(Table::Grades) поверх той же временной SQLite,
//    как в ownership_guard_test.cpp для applications);
//  - exams (шаг 2) — session_plan пользователя (02 §5.2 «Экран „Сессия“»),
//    все kind из CHECK DDL, сортировка по starts_at, изоляция по user_id;
//  - пользователь без оценок — пустой список и «пустое» состояние сводки
//    (average=null, disciplines=[]) — Аня, С5.6/03 §4.4;
//  - ошибка БД — fail-closed (ok=false → 500 контроллером), не пустой ответ.
// Данные копируют sql/fixtures.sql дословно: оценки user 3 (Марк) — 10 строк
// (ids 1–10; факт проверен sqlite по фикстурам: в постановке шага 1 было «8
// записей», в sql/fixtures.sql их 10 — все user_id=3, включая неуд «История
// МЭО» id 10), session_plan — 5 строк (у Марка retake 18.11, консультация и
// экзамен по МЭО; у Дины — дисциплина 10) + помеченные дополнения теста:
// id 99 grades и id 99 session_plan пользователя 9 (изоляция).
// Временная SQLite через настоящий DbClient — как в тестах Э2/Э3.
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/GradesService.h"
#include "services/OwnershipGuard.h"

namespace fs = std::filesystem;
using para::GradesService;
using para::OwnershipGuard;

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
               ("para-grades-" + std::to_string(stamp) + "-" +
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

// Схема — колонки таблиц disciplines/grades из миграции 0002, БЕЗ FK
// (FK-ссылки тест не проверяет — это работа миграций) и без полноты CHECK.
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withTables = true)
    {
        if (withTables)
            createSchema();
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }

    void createSchema()
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        exec(raw,
             "CREATE TABLE disciplines (id INTEGER PRIMARY KEY, code TEXT NOT"
             " NULL, title TEXT NOT NULL, control_type TEXT NOT NULL);"
             "INSERT INTO disciplines VALUES"
             " (1,'ME-101','Мировая экономика','exam'),"
             " (2,'PH-101','Философия','exam'),"
             " (3,'HME-101','История МЭО','exam'),"
             " (6,'MT-401','Международная торговля','graded_credit'),"
             " (7,'EC-401','Эконометрика','exam'),"
             " (8,'BE-401','Деловой английский','credit'),"
             " (9,'MN-401','Международные переговоры','exam'),"
             " (10,'IL-401','Международное право','exam');"
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
             "CREATE INDEX idx_grades_user ON grades(user_id, discipline_id);");
        // Оценки sql/fixtures.sql дословно (ids 1–10, все user_id=3 — Марк):
        // «Мировая экономика» 30+30+18=78 при пороге 82 (не хватает 4),
        // «История МЭО» — неуд, пересдача 18.11 (01, С4).
        exec(raw,
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
        // Дополнение теста (в sql/fixtures.sql оценок у user 9 нет): одна
        // строка другого пользователя — изоляция «только свои».
        exec(raw,
             "INSERT INTO grades(id, user_id, discipline_id, control_type,"
             " component, score, max_score, attempt, comment, graded_at)"
             " VALUES"
             " (99, 9, 1, 'exam', 'модуль 1', 30, 30, 1, NULL,"
             " '2026-10-05T12:00:00Z');");
        // session_plan (миграция 0002) — строки sql/fixtures.sql дословно:
        // Марк — пересдача «История МЭО» 18.11, консультация и экзамен МЭО
        // (С4.5); Дина — консультация и экзамен по международному праву.
        exec(raw,
             "CREATE TABLE session_plan ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id INTEGER NOT NULL,"
             "  discipline_id INTEGER NOT NULL,"
             "  kind TEXT NOT NULL,"
             "  starts_at TEXT NOT NULL,"
             "  room TEXT NOT NULL);"
             "CREATE INDEX idx_session_plan_user ON session_plan(user_id,"
             " starts_at);"
             "INSERT INTO session_plan(id, user_id, discipline_id, kind,"
             " starts_at, room) VALUES"
             " (1, 3, 3,  'retake',       '2026-11-18T10:00:00Z', '1108'),"
             " (2, 3, 1,  'consultation', '2026-11-21T10:00:00Z', '4210'),"
             " (3, 3, 1,  'exam',         '2026-11-24T10:00:00Z', '4210'),"
             " (4, 4, 10, 'consultation', '2026-11-21T14:00:00Z', '2210'),"
             " (5, 4, 10, 'exam',         '2026-11-25T10:00:00Z', '2210');");
        // Дополнение теста: session_plan другого пользователя (user 9).
        exec(raw,
             "INSERT INTO session_plan(id, user_id, discipline_id, kind,"
             " starts_at, room)"
             " VALUES (99, 9, 1, 'credit', '2026-11-20T10:00:00Z', '1201');");
        sqlite3_close(raw);
    }
};

GradesService::Result listOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::list(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
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
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

const Json::Value* disciplineOf(const Json::Value& arr, int64_t disciplineId)
{
    for (const auto& d : arr)
        if (d["discipline_id"].as<int64_t>() == disciplineId)
            return &d;
    return nullptr;
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
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

GradesService::Result examsOf(const drogon::orm::DbClientPtr& db,
                              int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::exams(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

OwnershipGuard::Verdict guardGrade(const drogon::orm::DbClientPtr& db,
                                   int64_t sessionUserId,
                                   int64_t gradeId)
{
    // Сценарий контроллера GET /v1/grades/{id}: guard ДО любых данных.
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db,
                          OwnershipGuard::Table::Grades,
                          sessionUserId,
                          gradeId,
                          [&p](OwnershipGuard::Verdict v) {
                              p.set_value(v);
                          });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

}  // namespace

TEST_CASE("GradesService: у пользователя без оценок — пустой список и пустая сводка")
{
    Fixture f;
    // Аня (user 1): в фикстурах оценок нет — «оценки появятся после первой
    // аттестации» (01 С5.6, 03 §4.4); пользователь без профиля/оценок — так же.
    const auto l = listOf(f.db, 1);
    REQUIRE(l.ok);
    REQUIRE(l.body.isMember("grades"));
    CHECK(l.body["grades"].size() == 0);

    const auto s = summaryOf(f.db, 1);
    REQUIRE(s.ok);
    // «Пустое» состояние сводки: average=null (не 0!) и пустой disciplines.
    CHECK(s.body["average"].isNull());
    CHECK(s.body["disciplines"].size() == 0);
    // И пользователь, у которого вообще нет строк в grades (user 4 — Дина).
    const auto s4 = summaryOf(f.db, 4);
    REQUIRE(s4.ok);
    CHECK(s4.body["average"].isNull());
    CHECK(s4.body["disciplines"].size() == 0);
}

TEST_CASE("GradesService: список — свои 10 строк фикстур, изоляция, поля строки")
{
    Fixture f;
    const auto l = listOf(f.db, 3);  // Марк
    REQUIRE(l.ok);
    const auto& grades = l.body["grades"];
    // Факт по sql/fixtures.sql (проверен sqlite): строк user_id=3 — 10
    // (в постановке шага названо «8» — расхождение постановки с фикстурами).
    CHECK(grades.size() == 10);
    std::set<int64_t> ids;
    for (const auto& g : grades)
        ids.insert(g["id"].as<int64_t>());
    CHECK(ids == std::set<int64_t>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    // Оценки другого пользователя (id 99, user 9) НЕ попали в список.
    CHECK(ids.count(99) == 0);

    // Поля строки — по id 1 (первая оценка «Мировой экономики»); позиция
    // в массиве зависит от ORDER BY d.title (первая — «Деловой английский»).
    for (const auto& g : grades)
    {
        if (g["id"].as<int64_t>() != 1)
            continue;
        CHECK(g["discipline_id"].as<int64_t>() == 1);
        CHECK(g["code"].asString() == "ME-101");
        CHECK(g["discipline"].asString() == "Мировая экономика");
        CHECK(g["control_type"].asString() == "exam");
        CHECK(g["component"].asString() == "модуль 1");
        CHECK(g["score"].as<int64_t>() == 30);
        CHECK(g["max_score"].as<int64_t>() == 30);
        CHECK(g["attempt"].as<int64_t>() == 1);
        CHECK(g["comment"].isNull());
        CHECK(g["graded_at"].asString() == "2026-10-05T12:00:00Z");
    }

    // Строка с комментарием преподавателя (неуд «История МЭО») — не null.
    for (const auto& g : grades)
    {
        if (g["id"].as<int64_t>() != 10)
            continue;
        CHECK(g["discipline"].asString() == "История МЭО");
        CHECK(g["score"].as<int64_t>() == 25);
        CHECK(g["max_score"].as<int64_t>() == 60);
        CHECK(g["comment"].asString() ==
              "Порог не достигнут; пересдача 18 ноября");
    }

    // Изоляция: у user 9 — только его строка id 99.
    const auto l9 = listOf(f.db, 9);
    REQUIRE(l9.ok);
    CHECK(l9.body["grades"].size() == 1);
    CHECK(l9.body["grades"][0]["id"].as<int64_t>() == 99);
}

TEST_CASE("GradesService: сводка Марка — 78/82 по дисциплине 1, средний 83")
{
    Fixture f;
    const auto s = summaryOf(f.db, 3);
    REQUIRE(s.ok);
    CHECK_FALSE(s.body["average"].isNull());

    // Дисциплин с оценками — 6 (фикстуры: d1, d3, d6, d7, d8, d9).
    const auto& arr = s.body["disciplines"];
    CHECK(arr.size() == 6);

    // «Мировая экономика»: 30+30+18=78 при пороге 82 — до допуска не
    // хватает 4 (демо-арифметика 05 §3 Э4, 03 §4.9.3); 100·78/82=95.1→95.
    const auto* d1 = disciplineOf(arr, 1);
    REQUIRE(d1 != nullptr);
    CHECK((*d1)["code"].asString() == "ME-101");
    CHECK((*d1)["discipline"].asString() == "Мировая экономика");
    CHECK((*d1)["control_type"].asString() == "exam");
    CHECK((*d1)["score_sum"].as<int64_t>() == 78);
    CHECK((*d1)["max_sum"].as<int64_t>() == 82);
    CHECK((*d1)["percent"].as<int>() == 95);
    CHECK((*d1)["missing"].as<int64_t>() == 4);

    // Закрытая дисциплина: 60/60 — missing 0, процент 100.
    const auto* d8 = disciplineOf(arr, 8);
    REQUIRE(d8 != nullptr);
    CHECK((*d8)["score_sum"].as<int64_t>() == 60);
    CHECK((*d8)["max_sum"].as<int64_t>() == 60);
    CHECK((*d8)["percent"].as<int>() == 100);
    CHECK((*d8)["missing"].as<int64_t>() == 0);

    // Неуд «История МЭО»: 25/60 → 42 % (<60 — danger, 03 §2.8), осталось 35.
    const auto* d3 = disciplineOf(arr, 3);
    REQUIRE(d3 != nullptr);
    CHECK((*d3)["score_sum"].as<int64_t>() == 25);
    CHECK((*d3)["max_sum"].as<int64_t>() == 60);
    CHECK((*d3)["percent"].as<int>() == 42);
    CHECK((*d3)["missing"].as<int64_t>() == 35);

    // Средний балл — взвешенно по всем дисциплинам: Σ score=293, Σ max=352,
    // 100·293/352 = 83.24 → 83 (склад значений из фикстур — сверено sqlite).
    CHECK(s.body["average"].as<int>() == 83);
}

TEST_CASE("GradesService: сводка по одной строке — average совпадает с ней")
{
    Fixture f;
    const auto s = summaryOf(f.db, 9);
    REQUIRE(s.ok);
    CHECK_FALSE(s.body["average"].isNull());
    CHECK(s.body["average"].as<int>() == 100);
    REQUIRE(s.body["disciplines"].size() == 1);
    CHECK(s.body["disciplines"][0]["discipline_id"].as<int64_t>() == 1);
    CHECK(s.body["disciplines"][0]["score_sum"].as<int64_t>() == 30);
    CHECK(s.body["disciplines"][0]["max_sum"].as<int64_t>() == 30);
    CHECK(s.body["disciplines"][0]["missing"].as<int64_t>() == 0);
}

TEST_CASE("GradesService: ошибка БД — fail-closed (ok=false → 500)")
{
    // Схема не создана: обе выборки падают — сервис отвечает ok=false
    // (контроллер переводит в 500 02 §5.1), а не «пустой успеваемостью».
    Fixture f(/*withTables=*/false);
    CHECK_FALSE(listOf(f.db, 3).ok);
    CHECK_FALSE(summaryOf(f.db, 3).ok);
    CHECK_FALSE(examsOf(f.db, 3).ok);
    CHECK(gradeByIdOf(f.db, 1).status == GradesService::One::Error);
}

TEST_CASE("OwnershipGuard (Table::Grades): своё/чужое/несуществующее/ошибка БД")
{
    // Сценарии контроллера GET /v1/grades/{id} (02 §6.3: guard различает
    // 403/404; путь контроллера: Own → данные, NotFound → 404, Forbidden →
    // 403, Error → 500). Оценки фикстур принадлежат user 3 (Марк).
    Fixture f;
    // Своя — grade id 1 у Марка.
    CHECK(guardGrade(f.db, 3, 1) == OwnershipGuard::Verdict::Own);
    // Чужая существующая: Аня (user 1) и пользователь 9 запрашивают grade
    // Марка → Forbidden (403), а не 404.
    CHECK(guardGrade(f.db, 1, 1) == OwnershipGuard::Verdict::Forbidden);
    CHECK(guardGrade(f.db, 9, 10) == OwnershipGuard::Verdict::Forbidden);
    // Несуществующая → NotFound (404) — для любого пользователя.
    CHECK(guardGrade(f.db, 3, 999) == OwnershipGuard::Verdict::NotFound);
    CHECK(guardGrade(f.db, 1, 999) == OwnershipGuard::Verdict::NotFound);
    // Ошибка БД — fail-closed: Error (контроллер → 500), не разрешение.
    Fixture broken(/*withTables=*/false);
    CHECK(guardGrade(broken.db, 3, 1) == OwnershipGuard::Verdict::Error);
}

TEST_CASE("GradesService::gradeById — одна запись по PK, NotFound отсутствующей")
{
    Fixture f;
    // Своя запись (после Own guard'а): те же поля, что в списке.
    const auto own = gradeByIdOf(f.db, 1);
    REQUIRE(own.status == GradesService::One::Found);
    CHECK(own.body["id"].as<int64_t>() == 1);
    CHECK(own.body["discipline_id"].as<int64_t>() == 1);
    CHECK(own.body["code"].asString() == "ME-101");
    CHECK(own.body["discipline"].asString() == "Мировая экономика");
    CHECK(own.body["component"].asString() == "модуль 1");
    CHECK(own.body["score"].as<int64_t>() == 30);
    CHECK(own.body["max_score"].as<int64_t>() == 30);
    CHECK(own.body["comment"].isNull());
    CHECK(own.body["graded_at"].asString() == "2026-10-05T12:00:00Z");

    // Запись с комментарием преподавателя (неуд «История МЭО»).
    const auto ret = gradeByIdOf(f.db, 10);
    REQUIRE(ret.status == GradesService::One::Found);
    CHECK(ret.body["comment"].asString() ==
          "Порог не достигнут; пересдача 18 ноября");
    CHECK(ret.body["score"].as<int64_t>() == 25);

    // Несуществующая → NotFound (контроллер → 404 02 §5.1).
    CHECK(gradeByIdOf(f.db, 999).status == GradesService::One::NotFound);
    CHECK(gradeByIdOf(f.db, -1).status == GradesService::One::NotFound);
    // Ошибка БД — Error (контроллер → 500), не NotFound.
    Fixture broken(/*withTables=*/false);
    CHECK(gradeByIdOf(broken.db, 1).status == GradesService::One::Error);
}

TEST_CASE("GradesService::exams — session_plan пользователя, порядок, изоляция")
{
    Fixture f;
    // Марк: 3 строки фикстур, по starts_at — retake 18.11 → консультация
    // 21.11 → экзамен 24.11; все kind из CHECK DDL попадают без фильтра.
    const auto m = examsOf(f.db, 3);
    REQUIRE(m.ok);
    const auto& arr = m.body["exams"];
    REQUIRE(arr.size() == 3);
    CHECK(arr[0]["id"].as<int64_t>() == 1);
    CHECK(arr[0]["kind"].asString() == "retake");  // пересдача — 05 §3 Э4
    CHECK(arr[0]["discipline"].asString() == "История МЭО");
    CHECK(arr[0]["code"].asString() == "HME-101");
    CHECK(arr[0]["room"].asString() == "1108");
    CHECK(arr[0]["starts_at"].asString() == "2026-11-18T10:00:00Z");
    CHECK(arr[1]["kind"].asString() == "consultation");
    CHECK(arr[1]["discipline"].asString() == "Мировая экономика");
    CHECK(arr[1]["room"].asString() == "4210");
    CHECK(arr[2]["kind"].asString() == "exam");
    CHECK(arr[2]["starts_at"].asString() == "2026-11-24T10:00:00Z");

    // Дина: только её две строки (discipline 10), чужие (Марк/9) не попали.
    const auto d = examsOf(f.db, 4);
    REQUIRE(d.ok);
    REQUIRE(d.body["exams"].size() == 2);
    CHECK(d.body["exams"][0]["discipline"].asString() == "Международное право");
    CHECK(d.body["exams"][0]["kind"].asString() == "consultation");

    // Аня (user 1) и пользователь 9 без session_plan фикстур... user 9 —
    // только его тестовая строка id 99 (изоляция).
    const auto a = examsOf(f.db, 1);
    REQUIRE(a.ok);
    CHECK(a.body["exams"].size() == 0);
    const auto nine = examsOf(f.db, 9);
    REQUIRE(nine.ok);
    REQUIRE(nine.body["exams"].size() == 1);
    CHECK(nine.body["exams"][0]["id"].as<int64_t>() == 99);
    CHECK(nine.body["exams"][0]["kind"].asString() == "credit");
}
