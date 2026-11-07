// Юнит-тесты GradebookService/RatingService (Э6а шаг 3; 02 §5.2:
// GET /v1/gradebook — «дисциплины с итогами, средний балл, счётчики»,
// GET /v1/rating — «рейтинг факультета по среднему баллу; чужие имена
// маскируются на сервере»):
//  - gradebook — агрегат «сумма score/max по дисциплине» (сверка с
//    фикстурами Марка: «Мировая экономика» 78/82, percent 95, average 83,
//    6 дисциплин, 10 работ), пустое состояние (Аня) — average null и
//    counters 0/0; пересдача (attempt 2) суммируется с первой попыткой
//    (постановка: «сумма score/max по дисциплине» — поведение зафиксировано);
//  - rating — факультет студента ИЗ БД (query не подменяет), своя строка
//    полная, чужие — «И. Фамилия» (NameMask; полное имя чужих в ответе
//    отсутствует); студенты без оценок — в конце с average null; staff —
//    все маскированы, query faculty применяется только ему; limit — после
//    сортировки; сортировка оценённых по среднему DESC;
//  - fail-closed: нет таблицы grades → Error (LEFT JOIN не «спасает»).
// Данные — срез sql/fixtures.sql: оценки Марка (10 строк, «Мировая
// экономика» 30+30+18=78 при пороге 82, «История МЭО» 25/60), группы
// 1-ФМО (Аня, Олег), 4-МЭО (Марк), 4-ФП (Дина).
#include <chrono>
#include <filesystem>
#include <future>
#include <optional>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>
#include <drogon/drogon.h>

#include "services/GradebookService.h"
#include "services/PeopleService.h"
#include "services/RatingService.h"

namespace fs = std::filesystem;
using para::GradebookService;
using para::PeopleService;
using para::RatingService;

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
               ("para-gbook-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void execFile(sqlite3* db, const std::string& sql)
{
    char* err = nullptr;
    REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
    if (err != nullptr)
    {
        FAIL(err);
        sqlite3_free(err);
    }
}

// Доп-вставки в уже открытую Fixture-БД (отдельное соединение по пути).
void execOnDb(const std::string& path, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(path.c_str(),
                            &raw,
                            SQLITE_OPEN_READWRITE,
                            nullptr) == SQLITE_OK);
    execFile(raw, sql);
    sqlite3_close(raw);
}

// Схема users/groups/profiles (0001), disciplines/grades (0002) — колонки,
// на которые опирается SQL сервисов; данные — срез фикстур.
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withGrades = true)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        execFile(raw,
                 "CREATE TABLE users ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  login TEXT NOT NULL UNIQUE,"
                 "  role TEXT NOT NULL CHECK (role IN ('student','staff')));"
                 "CREATE TABLE groups ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL UNIQUE,"
                 "  course INTEGER NOT NULL);"
                 "CREATE TABLE profiles ("
                 "  user_id INTEGER PRIMARY KEY REFERENCES users(id),"
                 "  fio TEXT NOT NULL,"
                 "  faculty TEXT NOT NULL,"
                 "  program TEXT NOT NULL,"
                 "  course INTEGER NOT NULL,"
                 "  group_id INTEGER NOT NULL REFERENCES groups(id),"
                 "  student_card_no TEXT NOT NULL UNIQUE);"
                 "CREATE TABLE disciplines ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  code TEXT NOT NULL UNIQUE,"
                 "  title TEXT NOT NULL,"
                 "  control_type TEXT NOT NULL);"
                 "CREATE TABLE grades ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE"
                 " CASCADE,"
                 "  discipline_id INTEGER NOT NULL REFERENCES"
                 " disciplines(id),"
                 "  control_type TEXT NOT NULL,"
                 "  component TEXT NOT NULL,"
                 "  score INTEGER NOT NULL,"
                 "  max_score INTEGER NOT NULL,"
                 "  attempt INTEGER NOT NULL DEFAULT 1,"
                 "  comment TEXT,"
                 "  graded_at TEXT NOT NULL);"
                 "INSERT INTO users(id, login, role) VALUES"
                 " (1,'anya.demo','student'), (2,'oleg.demo','student'),"
                 " (3,'mark.demo','student'), (4,'dina.demo','student'),"
                 " (5,'dean.demo','staff');"
                 "INSERT INTO groups(id, name, course) VALUES"
                 " (1,'1-ФМО',1), (2,'4-МЭО',4), (3,'4-ФП',4);"
                 "INSERT INTO profiles(user_id, fio, faculty, program,"
                 " course, group_id, student_card_no) VALUES"
                 " (1,'Соколова Анна Игоревна','Факультет международных"
                 " отношений (ФМО)','Международные отношения',1,1,"
                 " 'ФМО-101-26/001'),"
                 " (2,'Родионов Олег Павлович','Факультет международных"
                 " отношений (ФМО)','Международные отношения',1,1,"
                 " 'ФМО-101-26/042'),"
                 " (3,'Ливанов Марк Сергеевич','Факультет мировой экономики"
                 " и политики','Мировая экономика',4,2,'МЭО-401-23/017'),"
                 " (4,'Хакимова Дина Ринатовна','Факультет политологии"
                 " (ФП)','Сравнительная политология',4,3,'ФП-401-23/005');"
                 "INSERT INTO disciplines(id, code, title, control_type)"
                 " VALUES"
                 " (1,'ME-101','Мировая экономика','exam'),"
                 " (3,'HME-101','История МЭО','exam'),"
                 " (6,'MT-401','Международная торговля','graded_credit'),"
                 " (7,'EC-401','Эконометрика','exam'),"
                 " (8,'BE-401','Деловой английский','credit'),"
                 " (9,'MN-401','Международные переговоры','exam');");
        if (withGrades)
        {
            // Срез фикстур Марка (sql/fixtures.sql:88–98): 6 дисциплин,
            // 10 работ; «Мировая экономика» 30+30+18=78/82.
            execFile(raw,
                     "INSERT INTO grades(id, user_id, discipline_id,"
                     " control_type, component, score, max_score, attempt,"
                     " comment, graded_at) VALUES"
                     " (1,3,1,'exam','модуль 1',30,30,1,NULL,"
                     "  '2026-10-05T12:00:00Z'),"
                     " (2,3,1,'exam','модуль 2',30,30,1,NULL,"
                     "  '2026-10-19T12:00:00Z'),"
                     " (3,3,1,'exam','семинары',18,22,1,NULL,"
                     "  '2026-10-31T12:00:00Z'),"
                     " (4,3,6,'graded_credit','модуль 1',27,30,1,NULL,"
                     "  '2026-10-08T12:00:00Z'),"
                     " (5,3,6,'graded_credit','реферат',28,30,1,NULL,"
                     "  '2026-10-22T12:00:00Z'),"
                     " (6,3,7,'exam','модуль 1',24,30,1,NULL,"
                     "  '2026-10-07T12:00:00Z'),"
                     " (7,3,7,'exam','модуль 2',26,30,1,NULL,"
                     "  '2026-10-21T12:00:00Z'),"
                     " (8,3,8,'credit','зачёт',60,60,1,NULL,"
                     "  '2026-10-14T12:00:00Z'),"
                     " (9,3,9,'exam','модуль 1',25,30,1,NULL,"
                     "  '2026-10-09T12:00:00Z'),"
                     " (10,3,3,'exam','экзамен',25,60,1,"
                     "  'Порог не достигнут; пересдача 18 ноября',"
                     "  '2026-06-24T12:00:00Z');");
        }
        sqlite3_close(raw);
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }
};

PeopleService::Requester requesterOf(Fixture& f, int64_t userId)
{
    std::promise<std::pair<PeopleService::Status, PeopleService::Requester>>
        p;
    auto fut = p.get_future();
    PeopleService::loadRequester(
        f.db, userId,
        [&p](PeopleService::Status s, PeopleService::Requester r) {
            p.set_value({s, std::move(r)});
        });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    auto [s, r] = fut.get();
    REQUIRE(s == PeopleService::Status::Ok);
    return r;
}

GradebookService::Result gradebookOf(Fixture& f, int64_t userId)
{
    std::promise<GradebookService::Result> p;
    auto fut = p.get_future();
    GradebookService::gradebook(
        f.db, userId,
        [&p](GradebookService::Result r) { p.set_value(std::move(r)); });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return fut.get();
}

RatingService::Result ratingOf(Fixture& f,
                               int64_t requesterId,
                               std::optional<std::string> faculty = {},
                               int maxRows = 0)
{
    auto req = requesterOf(f, requesterId);
    std::promise<RatingService::Result> p;
    auto fut = p.get_future();
    RatingService::rating(f.db, std::move(req), std::move(faculty), maxRows,
                          [&p](RatingService::Result r) {
                              p.set_value(std::move(r));
                          });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return fut.get();
}

}  // namespace

TEST_CASE("gradebook: агрегат по дисциплинам — сверка с фикстурами Марка")
{
    Fixture f;
    auto r = gradebookOf(f, 3);
    REQUIRE(r.status == GradebookService::Status::Ok);
    const auto& body = r.body;
    // Средний: Σscore 293 / Σmax 352 → 83 (взвешенный процент, целое — Э4).
    CHECK(body["average"].as<int64_t>() == 83);
    const auto& ds = body["disciplines"];
    REQUIRE(ds.size() == 6);  // фикстуры: 6 дисциплин с итогами
    // ORDER BY d.title: Деловой английский < История МЭО < Международная
    // торговля < Международные переговоры < Мировая экономика < Эконометрика.
    CHECK(ds[0]["code"].asString() == "BE-401");
    CHECK(ds[0]["score_sum"].as<int64_t>() == 60);
    CHECK(ds[0]["max_sum"].as<int64_t>() == 60);
    CHECK(ds[0]["percent"].as<int64_t>() == 100);
    CHECK(ds[0]["works"].as<int64_t>() == 1);
    // «Мировая экономика» (постановка шага: 78/82 по дисциплине 1):
    // 30+30+18=78 при пороге 82 → percent 95.
    CHECK(ds[4]["discipline_id"].as<int64_t>() == 1);
    CHECK(ds[4]["code"].asString() == "ME-101");
    CHECK(ds[4]["discipline"].asString() == "Мировая экономика");
    CHECK(ds[4]["control_type"].asString() == "exam");
    CHECK(ds[4]["score_sum"].as<int64_t>() == 78);
    CHECK(ds[4]["max_sum"].as<int64_t>() == 82);
    CHECK(ds[4]["percent"].as<int64_t>() == 95);
    CHECK(ds[4]["works"].as<int64_t>() == 3);
    // Счётчики: 6 дисциплин, 10 работ (фикстуры).
    CHECK(body["counters"]["disciplines"].as<int64_t>() == 6);
    CHECK(body["counters"]["works"].as<int64_t>() == 10);
    // «История МЭО»: 25/60 с комментарием о пересдаче — сумма строк.
    bool found = false;
    for (const auto& d : ds)
        if (d["discipline_id"].as<int64_t>() == 3)
        {
            found = true;
            CHECK(d["score_sum"].as<int64_t>() == 25);
            CHECK(d["max_sum"].as<int64_t>() == 60);
        }
    CHECK(found);
}

TEST_CASE("gradebook: пустое состояние (Аня) — average null, counters 0")
{
    Fixture f;
    auto r = gradebookOf(f, 1);
    REQUIRE(r.status == GradebookService::Status::Ok);
    CHECK(r.body["average"].isNull());
    CHECK(r.body["disciplines"].size() == 0);
    CHECK(r.body["counters"]["disciplines"].as<int64_t>() == 0);
    CHECK(r.body["counters"]["works"].as<int64_t>() == 0);
}

TEST_CASE("gradebook: пересдача (attempt 2) суммируется — «сумма score/max»")
{
    Fixture f;
    // Дополнение теста (в фикстурах пересдач нет): вторая попытка экзамена
    // «История МЭО» — постановка шага: итог дисциплины = сумма score/max
    // всех строк (25+40=65 из 60+60=120), поведение зафиксировано.
    execOnDb(f.dbPath,
                   "INSERT INTO grades(id, user_id, discipline_id,"
                   " control_type, component, score, max_score, attempt,"
                   " comment, graded_at) VALUES"
                   " (11,3,3,'exam','экзамен',40,60,2,"
                   "  'Пересдача 18 ноября','2026-11-18T14:00:00Z');");
    auto r = gradebookOf(f, 3);
    REQUIRE(r.status == GradebookService::Status::Ok);
    for (const auto& d : r.body["disciplines"])
        if (d["discipline_id"].as<int64_t>() == 3)
        {
            CHECK(d["score_sum"].as<int64_t>() == 65);   // 25 + 40
            CHECK(d["max_sum"].as<int64_t>() == 120);    // 60 + 60
            CHECK(d["works"].as<int64_t>() == 2);
        }
    // Средний пересчитался: Σscore 293+40=333, Σmax 352+60=412 → 81.
    CHECK(r.body["average"].as<int64_t>() == 81);
    CHECK(r.body["counters"]["works"].as<int64_t>() == 11);
}

TEST_CASE("rating: своя строка полная, чужая — маска (02 §5.2)")
{
    Fixture f;
    // Марк (МЭО): факультет из БД, своя строка — полное ФИО.
    auto r = ratingOf(f, 3);
    REQUIRE(r.status == RatingService::Status::Ok);
    CHECK(r.body["faculty"].asString() ==
          "Факультет мировой экономики и политики");
    const auto& lines = r.body["rating"];
    REQUIRE(lines.size() == 1);
    CHECK(lines[0]["rank"].as<int64_t>() == 1);
    CHECK(lines[0]["user_id"].as<int64_t>() == 3);
    CHECK(lines[0]["fio"].asString() == "Ливанов Марк Сергеевич");
    CHECK(lines[0]["average"].as<int64_t>() == 83);
    CHECK(lines[0]["self"].asBool() == true);
    CHECK(r.body["masked"].asBool() == true);

    // Аня (ФМО): своя строка полная, Олег — «О. Родионов» (маска; обе
    // average null — оценок нет, в конце списка).
    auto ra = ratingOf(f, 1);
    REQUIRE(ra.status == RatingService::Status::Ok);
    CHECK(ra.body["faculty"].asString() ==
          "Факультет международных отношений (ФМО)");
    const auto& fl = ra.body["rating"];
    REQUIRE(fl.size() == 2);
    CHECK(fl[0]["user_id"].as<int64_t>() == 1);
    CHECK(fl[0]["fio"].asString() == "Соколова Анна Игоревна");
    CHECK(fl[0]["average"].isNull());
    CHECK(fl[0]["self"].asBool() == true);
    CHECK(fl[1]["user_id"].as<int64_t>() == 2);
    CHECK(fl[1]["fio"].asString() == "О. Родионов");
    CHECK(fl[1]["average"].isNull());
    CHECK(fl[1]["self"].asBool() == false);
    // Полное имя чужого наружу не уходит вовсе: сериализованный ответ
    // (emitUTF8 — сырая кириллица, как api::jsonResponse) содержит маску
    // «О. Родионов», но НЕ полное «Родионов Олег Павлович».
    Json::StreamWriterBuilder b;
    b["emitUTF8"] = true;
    b["indentation"] = "";
    const std::string raw = Json::writeString(b, ra.body);
    CHECK(raw.find("О. Родионов") != std::string::npos);
    CHECK(raw.find("Родионов Олег Павлович") == std::string::npos);
}

TEST_CASE("rating: staff — все факультеты (или ?faculty=), все маскированы; "
          "сортировка оценённых по среднему")
{
    Fixture f;
    // dean без faculty: все 4 студента; оценённый Марк первый, остальные
    // (null) по user_id; ВСЕ имена маскированы — у staff нет «своей» строки.
    auto r = ratingOf(f, 5);
    REQUIRE(r.status == RatingService::Status::Ok);
    CHECK(r.body["faculty"].isNull());
    const auto& lines = r.body["rating"];
    REQUIRE(lines.size() == 4);
    CHECK(lines[0]["user_id"].as<int64_t>() == 3);
    CHECK(lines[0]["fio"].asString() == "М. Ливанов");
    CHECK(lines[0]["average"].as<int64_t>() == 83);
    CHECK(lines[0]["self"].asBool() == false);
    CHECK(lines[1]["fio"].asString() == "А. Соколова");
    CHECK(lines[2]["fio"].asString() == "О. Родионов");
    CHECK(lines[3]["fio"].asString() == "Д. Хакимова");
    for (const auto& l : lines)
        CHECK(l["self"].asBool() == false);

    // staff + faculty-фильтр — только ФМО.
    auto rf = ratingOf(f, 5,
                       std::string("Факультет международных отношений (ФМО)"));
    REQUIRE(rf.status == RatingService::Status::Ok);
    CHECK(rf.body["faculty"].asString() ==
          "Факультет международных отношений (ФМО)");
    CHECK(rf.body["rating"].size() == 2);

    // Студент НЕ может открыть чужой факультет через query (Р-15-стиль):
    // Аня с faculty=МЭО всё равно видит рейтинг ФМО.
    auto rw = ratingOf(f, 1,
                       std::string("Факультет мировой экономики и политики"));
    REQUIRE(rw.status == RatingService::Status::Ok);
    CHECK(rw.body["faculty"].asString() ==
          "Факультет международных отношений (ФМО)");
    CHECK(rw.body["rating"].size() == 2);
}

TEST_CASE("rating: сортировка оценённых по среднему; limit — после сортировки")
{
    Fixture f;
    // Аня 60/100, Олег 50/100: оценённые по среднему DESC — Аня выше Олега;
    // Марк (83) выше обеих. dean видит: Марк 83, Аня 60, Олег 50, Дина null.
    execOnDb(f.dbPath,
                   "INSERT INTO grades(id, user_id, discipline_id,"
                   " control_type, component, score, max_score, attempt,"
                   " comment, graded_at) VALUES"
                   " (12,1,1,'exam','модуль 1',60,100,1,NULL,"
                   "  '2026-10-05T12:00:00Z'),"
                   " (13,2,1,'exam','модуль 1',50,100,1,NULL,"
                   "  '2026-10-05T12:00:00Z');");
    auto r = ratingOf(f, 5);
    REQUIRE(r.status == RatingService::Status::Ok);
    const auto& lines = r.body["rating"];
    REQUIRE(lines.size() == 4);
    CHECK(lines[0]["user_id"].as<int64_t>() == 3);   // 83
    CHECK(lines[0]["rank"].as<int64_t>() == 1);
    CHECK(lines[1]["user_id"].as<int64_t>() == 1);   // 60
    CHECK(lines[1]["average"].as<int64_t>() == 60);
    CHECK(lines[1]["rank"].as<int64_t>() == 2);
    CHECK(lines[2]["user_id"].as<int64_t>() == 2);   // 50
    CHECK(lines[2]["rank"].as<int64_t>() == 3);
    CHECK(lines[3]["user_id"].as<int64_t>() == 4);   // null — конец списка
    CHECK(lines[3]["average"].isNull());
    CHECK(lines[3]["rank"].as<int64_t>() == 4);

    // limit=2 — первые две строки отсортированного списка.
    auto rl = ratingOf(f, 5, {}, 2);
    REQUIRE(rl.status == RatingService::Status::Ok);
    REQUIRE(rl.body["rating"].size() == 2);
    CHECK(rl.body["rating"][0]["user_id"].as<int64_t>() == 3);
    CHECK(rl.body["rating"][1]["user_id"].as<int64_t>() == 1);
}

TEST_CASE("fail-closed: нет таблицы grades → Error (не пустой список)")
{
    Fixture f;
    // Fixture создаёт (пустую) таблицу grades — «Ok-пусто» на пустой таблице
    // уже проверено кейсом Ани; здесь честный fail-closed: таблицы НЕТ.
    execOnDb(f.dbPath, "DROP TABLE grades;");
    auto gb = gradebookOf(f, 3);
    CHECK(gb.status == GradebookService::Status::Error);

    auto r = ratingOf(f, 3);
    CHECK(r.status == RatingService::Status::Error);
}
