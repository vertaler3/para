// Юнит-тесты LibraryService (Э6б шаг 2; 02 §5.2 строки /v1/library*,
// миграция 0006):
//  - branches — филиалы с корпусом (JOIN buildings), порядок по id; пустая
//    таблица — 200 {"branches":[]}, а не ошибка;
//  - items — наличие литературы: сортировка по title; контрольная строка
//    «свободно 2 из 3» (total=3/available=2, филиал «Главная библиотека»,
//    зал 2 — проверка гайда §9б); фильтр available=1 отсекает available=0
//    (meaningful: в наборе ровно одна такая книга, id 2); фильтр branch_id;
//    поиск q по title И author (LIKE '%q%'); пустой результат — 200 [];
//  - my-card — билет ТЕКУЩЕГО пользователя: card_no + ФИО/группа из своего
//    профиля; у staff-билета (профиль не заводится — 02 §4.1) fio/group —
//    null; без билета → NotFound (404, 02 §5.2 «404 (нет билета)»);
//  - ошибка БД — fail-closed (Error/ok=false → 500 контроллером), не
//    пустые списки.
// Данные копируют sql/fixtures.sql Э6б дословно (2 филиала в разных
// корпусах; контрольные книги id 1 «Основы макроэкономики» 3/2 и id 2
// «Вестник…» available=0) + помеченные дополнения теста: книги 3–5 и
// пользователь 6 без билета (путь 404). Временная SQLite через настоящий
// DbClient — как в тестах Э5а/Э6а.
//
// ОГРАНИЧЕНИЕ (задокументировано в журнале шага): LIKE в SQLite без ICU
// регистрозависим для не-ASCII — поиск по кириллице работает по точному
// регистру подстроки («Гущин» найдёт, «гущин» — нет). Тест фиксирует
// поведение на точном регистре; контракт 02 §5.2 регистр не оговаривает.
#include <chrono>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>
#include <drogon/drogon.h>

#include "services/LibraryService.h"

namespace fs = std::filesystem;
using para::LibraryService;

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
               ("para-library-" + std::to_string(stamp) + "-" +
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

// Схема — колонки 0006 (buildings/library_branches/library_items/
// library_cards) + users/profiles/groups (FK-якоря my-card). UNIQUE
// library_cards.card_no включён — контракт «билеты уникальны».
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    // withTables=false — ПУСТАЯ БД без схемы: fail-closed-кейс (выборки
    // падают — сервисы обязаны отвечать Error/ok=false, 500 контроллером),
    // как withTables в тестах RoomsService. withData=false — схема есть,
    // данных нет (пустые каталоги — 200, не ошибка).
    explicit Fixture(bool withData = true, bool withTables = true)
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        if (withTables)
        {
            exec(raw,
             "CREATE TABLE users (id INTEGER PRIMARY KEY, login TEXT NOT"
             " NULL, role TEXT NOT NULL);"
             "CREATE TABLE groups (id INTEGER PRIMARY KEY, name TEXT NOT"
             " NULL, course INTEGER NOT NULL);"
             "CREATE TABLE profiles ("
             "  user_id INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE"
             " CASCADE,"
             "  fio TEXT NOT NULL, faculty TEXT, program TEXT,"
             "  course INTEGER, group_id INTEGER REFERENCES groups(id),"
             "  student_card_no TEXT);"
             "CREATE TABLE buildings ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  name TEXT NOT NULL, address TEXT NOT NULL);"
             "CREATE TABLE library_branches ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  name TEXT NOT NULL,"
             "  building_id INTEGER NOT NULL REFERENCES buildings(id));"
             "CREATE TABLE library_items ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  title TEXT NOT NULL, author TEXT,"
             "  kind TEXT NOT NULL DEFAULT 'book',"
             "  branch_id INTEGER NOT NULL REFERENCES library_branches(id),"
             "  hall TEXT, total INTEGER NOT NULL DEFAULT 1,"
             "  available INTEGER NOT NULL DEFAULT 1);"
             "CREATE TABLE library_cards ("
             "  user_id INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE"
             " CASCADE,"
             "  card_no TEXT NOT NULL UNIQUE);"
             "CREATE INDEX idx_library_items_branch ON"
             " library_items(branch_id, available);");
        }
        if (withData)
        {
            // users: 1/2 — студенты, 5 — staff (билет есть, профиля нет —
            // как dean.demo), 6 — дополнение теста (БЕЗ билета).
            exec(raw,
                 "INSERT INTO users(id, login, role) VALUES"
                 " (1,'anya.demo','student'), (2,'oleg.demo','student'),"
                 " (5,'dean.demo','staff'), (6,'office.demo','staff');"
                 "INSERT INTO groups(id, name, course) VALUES (1,'1-ФМО',1);"
                 "INSERT INTO profiles(user_id, fio, faculty, program,"
                 " course, group_id, student_card_no) VALUES"
                 " (1,'Соколова Анна Игоревна','ФМО (вымышленный)',"
                 "  'Международные отношения',1,1,'ФМО-101-26/001');");
            // Корпуса и филиалы — строки sql/fixtures.sql Э6б.
            exec(raw,
                 "INSERT INTO buildings(id, name, address) VALUES"
                 " (1,'Корпус А','проспект Научный, д. 10 (адрес"
                 " вымышленный)'),"
                 " (2,'Корпус Б','улица Парковая, д. 4 (адрес"
                 " вымышленный)');"
                 "INSERT INTO library_branches(id, name, building_id)"
                 " VALUES (1,'Главная библиотека',1),"
                 " (2,'Библиотека ФМО',2);");
            // 5 книг: id 1 — контроль «свободно 2 из 3», id 2 —
            // available=0 для meaningful-фильтра; 3–5 — дополнения теста.
            exec(raw,
                 "INSERT INTO library_items(id, title, author, kind,"
                 " branch_id, hall, total, available) VALUES"
                 " (1,'Основы макроэкономики','Каменева Ольга Дмитриевна',"
                 "  'textbook',1,'зал 2, стеллаж А-4',3,2),"
                 " (2,'Вестник международных отношений',"
                 "  'Редколлегия (вымышленная)','journal',1,"
                 "  'зал 3, стеллаж П-1',1,0),"
                 " (3,'Теория международной торговли',"
                 "  'Гущин Артём Валерьевич','book',1,"
                 "  'зал 2, стеллаж А-9',2,2),"
                 " (4,'История МЭО: очерки','Рахимов Тимур Наилевич',"
                 "  'book',2,'зал 1, стеллаж А-2',3,3),"
                 " (5,'Политология: краткий курс',"
                 "  'Одинцова Елена Викторовна','textbook',2,"
                 "  'зал 1, стеллаж В-6',4,2);");
            // Билеты фикстур (Аня LB-2026-0001, декан LB-2026-0005); у
            // user 6 билета НЕТ (дополнение теста — путь 404 my-card).
            exec(raw,
                 "INSERT INTO library_cards(user_id, card_no) VALUES"
                 " (1,'LB-2026-0001'), (5,'LB-2026-0005');");
        }
        sqlite3_close(raw);
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }
};

LibraryService::Result branchesOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<LibraryService::Result> p;
    auto f = p.get_future();
    LibraryService::branches(db, [&p](LibraryService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

LibraryService::Result itemsOf(const drogon::orm::DbClientPtr& db,
                               LibraryService::ItemFilters filters = {})
{
    std::promise<LibraryService::Result> p;
    auto f = p.get_future();
    LibraryService::items(db, filters, [&p](LibraryService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

LibraryService::CardResult cardOf(const drogon::orm::DbClientPtr& db,
                                  int64_t userId)
{
    std::promise<LibraryService::CardResult> p;
    auto f = p.get_future();
    LibraryService::myCard(db, userId, [&p](LibraryService::CardResult r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// «список id» для проверок порядка и состава.
std::vector<int64_t> idsOf(const Json::Value& arr)
{
    std::vector<int64_t> out;
    for (const auto& it : arr)
        out.push_back(it["id"].as<int64_t>());
    return out;
}

}  // namespace

TEST_CASE("LibraryService::branches — филиалы с корпусом, порядок по id")
{
    Fixture fx;
    auto r = branchesOf(fx.db);
    REQUIRE(r.ok);
    const auto& arr = r.body["branches"];
    REQUIRE(arr.size() == 2);
    CHECK(arr[0]["id"].as<int64_t>() == 1);
    CHECK(arr[0]["name"].asString() == "Главная библиотека");
    CHECK(arr[0]["building_id"].as<int64_t>() == 1);
    CHECK(arr[0]["building"].asString() == "Корпус А");
    CHECK(arr[1]["id"].as<int64_t>() == 2);
    CHECK(arr[1]["name"].asString() == "Библиотека ФМО");
    CHECK(arr[1]["building_id"].as<int64_t>() == 2);
    CHECK(arr[1]["building"].asString() == "Корпус Б");
}

TEST_CASE("LibraryService::branches — пустая таблица → 200 {branches:[]}")
{
    Fixture fx(false);
    auto r = branchesOf(fx.db);
    REQUIRE(r.ok);
    CHECK(r.body["branches"].size() == 0);
}

TEST_CASE("LibraryService::items — сортировка по title, поля строки")
{
    Fixture fx;
    auto r = itemsOf(fx.db);
    REQUIRE(r.ok);
    const auto& arr = r.body["items"];
    REQUIRE(arr.size() == 5);
    // UTF-8 байтовый порядок: В < И < О < П < Т (монотонно по кодовым
    // точкам кириллицы) → ids 2,4,1,5,3.
    const auto ids = idsOf(arr);
    REQUIRE(ids.size() == 5);
    CHECK(ids[0] == 2);
    CHECK(ids[1] == 4);
    CHECK(ids[2] == 1);
    CHECK(ids[3] == 5);
    CHECK(ids[4] == 3);
    // Поля строки каталога (02 §5.2: наличие, филиал, зал).
    const auto& book = arr[2];  // id 1 — контрольная книга
    CHECK(book["title"].asString() == "Основы макроэкономики");
    CHECK(book["author"].asString() == "Каменева Ольга Дмитриевна");
    CHECK(book["kind"].asString() == "textbook");
    CHECK(book["branch_id"].as<int64_t>() == 1);
    CHECK(book["branch"].asString() == "Главная библиотека");
    CHECK(book["building"].asString() == "Корпус А");
    CHECK(book["hall"].asString() == "зал 2, стеллаж А-4");
}

TEST_CASE("LibraryService::items — контроль «свободно 2 из 3» (гайд §9б)")
{
    Fixture fx;
    auto r = itemsOf(fx.db);
    REQUIRE(r.ok);
    bool checked = false;
    for (const auto& it : r.body["items"])
    {
        if (it["id"].as<int64_t>() != 1)
            continue;
        CHECK(it["total"].as<int64_t>() == 3);
        CHECK(it["available"].as<int64_t>() == 2);  // «свободно 2 из 3»
        CHECK(it["branch"].asString() == "Главная библиотека");
        CHECK(it["hall"].asString() == "зал 2, стеллаж А-4");
        checked = true;
    }
    CHECK(checked);  // контрольная строка присутствует ровно одна
}

TEST_CASE("LibraryService::items — available=1 отсекает available=0")
{
    Fixture fx;
    LibraryService::ItemFilters f;
    f.availableOnly = true;
    auto r = itemsOf(fx.db, f);
    REQUIRE(r.ok);
    const auto ids = idsOf(r.body["items"]);
    REQUIRE(ids.size() == 4);  // из 5 — единственная available=0 (id 2) ушла
    for (int64_t id : ids)
        CHECK(id != 2);
    // И контрольная осталась.
    bool has1 = false;
    for (const auto& it : r.body["items"])
        if (it["id"].as<int64_t>() == 1)
            has1 = true;
    CHECK(has1);
}

TEST_CASE("LibraryService::items — фильтр branch_id")
{
    Fixture fx;
    LibraryService::ItemFilters f;
    f.branchId = 2;
    auto r = itemsOf(fx.db, f);
    REQUIRE(r.ok);
    const auto ids = idsOf(r.body["items"]);
    REQUIRE(ids.size() == 2);
    CHECK(ids[0] == 4);
    CHECK(ids[1] == 5);
    for (const auto& it : r.body["items"])
        CHECK(it["branch"].asString() == "Библиотека ФМО");
    // Несуществующий филиал — пустой результат, не ошибка.
    LibraryService::ItemFilters missing;
    missing.branchId = 999;
    auto r2 = itemsOf(fx.db, missing);
    REQUIRE(r2.ok);
    CHECK(r2.body["items"].size() == 0);
}

TEST_CASE("LibraryService::items — поиск q по title и author")
{
    Fixture fx;
    LibraryService::ItemFilters byTitle;
    byTitle.q = "макро";  // подстрока title («Основы макроэкономики»)
    auto r1 = itemsOf(fx.db, byTitle);
    REQUIRE(r1.ok);
    const auto ids1 = idsOf(r1.body["items"]);
    REQUIRE(ids1.size() == 1);
    CHECK(ids1[0] == 1);

    LibraryService::ItemFilters byAuthor;
    byAuthor.q = "Гущин";  // подстрока author (id 3)
    auto r2 = itemsOf(fx.db, byAuthor);
    REQUIRE(r2.ok);
    const auto ids2 = idsOf(r2.body["items"]);
    REQUIRE(ids2.size() == 1);
    CHECK(ids2[0] == 3);

    // Пустой q = без фильтра; без совпадений — 200 [] (не ошибка);
    // спецсимволы LIKE в q экранированы (поиск по подстроке, не шаблон:
    // «%» в q НЕ должен превращать фильтр в «всё подряд»).
    LibraryService::ItemFilters emptyQ;
    emptyQ.q = "";
    auto r3 = itemsOf(fx.db, emptyQ);
    REQUIRE(r3.ok);
    CHECK(r3.body["items"].size() == 5);

    LibraryService::ItemFilters none;
    none.q = "ззз-нет-такого";
    auto r4 = itemsOf(fx.db, none);
    REQUIRE(r4.ok);
    CHECK(r4.body["items"].size() == 0);

    LibraryService::ItemFilters wildcard;
    wildcard.q = "%";
    auto r5 = itemsOf(fx.db, wildcard);
    REQUIRE(r5.ok);
    CHECK(r5.body["items"].size() == 0);  // экранировано — совпадений нет
}

TEST_CASE("LibraryService::items — комбинированный фильтр")
{
    Fixture fx;
    LibraryService::ItemFilters f;
    f.q = "макро";
    f.branchId = 1;
    f.availableOnly = true;
    auto r = itemsOf(fx.db, f);
    REQUIRE(r.ok);
    const auto ids = idsOf(r.body["items"]);
    REQUIRE(ids.size() == 1);
    CHECK(ids[0] == 1);
}

TEST_CASE("LibraryService::myCard — свой билет с ФИО и группой")
{
    Fixture fx;
    auto r = cardOf(fx.db, 1);  // Аня: билет + профиль фикстур
    REQUIRE(r.status == LibraryService::CardStatus::Ok);
    CHECK(r.body["card_no"].asString() == "LB-2026-0001");
    CHECK(r.body["fio"].asString() == "Соколова Анна Игоревна");
    CHECK(r.body["group"].asString() == "1-ФМО");
}

TEST_CASE("LibraryService::myCard — staff-билет без профиля: fio/group null")
{
    Fixture fx;
    auto r = cardOf(fx.db, 5);  // декан: билет есть (фикстуры), профиля нет
    REQUIRE(r.status == LibraryService::CardStatus::Ok);
    CHECK(r.body["card_no"].asString() == "LB-2026-0005");
    CHECK(r.body["fio"].isNull());
    CHECK(r.body["group"].isNull());
}

TEST_CASE("LibraryService::myCard — нет билета → NotFound (404)")
{
    Fixture fx;
    auto r1 = cardOf(fx.db, 6);   // дополнение теста: пользователь без билета
    CHECK(r1.status == LibraryService::CardStatus::NotFound);
    auto r2 = cardOf(fx.db, 999);  // несуществующий пользователь
    CHECK(r2.status == LibraryService::CardStatus::NotFound);
}

TEST_CASE("LibraryService — ошибка БД — fail-closed (Error/ok=false)")
{
    // Схема не создана (withTables=false): выборки падают — сервис отвечает
    // Error (500 контроллером), не пустой список/NotFound.
    Fixture fx(false, false);
    auto r1 = branchesOf(fx.db);
    CHECK_FALSE(r1.ok);
    auto r2 = itemsOf(fx.db);
    CHECK_FALSE(r2.ok);
    auto r3 = cardOf(fx.db, 1);
    CHECK(r3.status == LibraryService::CardStatus::Error);
}
