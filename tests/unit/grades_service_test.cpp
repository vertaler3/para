// Юнит-тесты GradesService (Э4 шаг 1; 02 §5.2, миграция 0002):
//  - список — только СВОИ оценки (user_id из сессии), с кодом/названием
//    дисциплины, comment null у строк без комментария;
//  - сводка — агрегат по дисциплинам: порог = Σ max_score компонентов,
//    «что осталось» (missing) = max(0, Σ max_score − Σ score), средний балл —
//    взвешенный процент Σ score / Σ max_score × 100 (демо-арифметика
//    05 §3 Э4: у «Мировой экономики» 30+30+18=78 при пороге 82 — не хватает 4);
//  - пользователь без оценок — пустой список и «пустое» состояние сводки
//    (average=null, disciplines=[]) — Аня, С5.6/03 §4.4;
//  - ошибка БД — fail-closed (ok=false → 500 контроллером), не пустой ответ.
// Данные копируют оценки sql/fixtures.sql дословно: user 3 (Марк) — 10 строк
// (ids 1–10; факт проверен sqlite по фикстурам: в постановке шага было «8
// записей», в sql/fixtures.sql их 10 — все user_id=3, включая неуд «История
// МЭО» id 10) + помеченное дополнение теста: id 99 пользователя 9 (изоляция).
// Временная SQLite через настоящий DbClient — как в тестах Э2/Э3.
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/GradesService.h"

namespace fs = std::filesystem;
using para::GradesService;

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
}
