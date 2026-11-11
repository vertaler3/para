// Юнит-тесты CanteensService и MapService (Э6б шаг 3; 02 §5.2 строки
// /v1/canteens*, /v1/map*, миграция 0006):
//  - canteens — список с корпусом и часами; пустая таблица — 200 [];
//  - menu — цены/комплексные/КБЖУ: is_combo bool, kcal/protein/fat/carbs
//    nullable (вода без КБЖУ — null, fixtures Э6б); несуществующая
//    столовая → NotFound (404); пустое меню существующей — 200 [];
//  - map/buildings — корпуса;
//  - map/rooms — поиск кабинета по номеру: «1207» → Корпус Б / этаж 2 /
//    аудитория (контрольная строка гайда §9б и fixtures Э6а), подстрока
//    («204» → «Б-204»), фильтр building_id, комбо-фильтр, q без
//    совпадений — пусто (не ошибка), экранирование «%»;
//  - fail-closed — БД без схемы → Error/ok=false (не пустые списки).
// Данные копируют sql/fixtures.sql Э6а/Э6б дословно (2 корпуса, 3 аудитории
// с building_id/floor/kind, 2 столовые) + помеченные дополнения теста
// (компактное меню из контрольных строк Э6б). Временная SQLite через
// настоящий DbClient — как в тестах Э5а/Э6б шаг 2.
#include <chrono>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>
#include <drogon/drogon.h>

#include "services/CanteensService.h"
#include "services/MapService.h"

namespace fs = std::filesystem;
using para::CanteensService;
using para::MapService;

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
               ("para-campus-" + std::to_string(stamp) + "-" +
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

// Схема — колонки 0006 (buildings/canteens/menu_items) + rooms с колонками
// ALTER 0006 (building_id/floor/kind — DDL 0005 + ALTER).
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

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
                 "CREATE TABLE rooms ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL,"
                 "  building TEXT NOT NULL,"
                 "  capacity INTEGER NOT NULL,"
                 "  features TEXT NOT NULL DEFAULT '');"
                 "ALTER TABLE rooms ADD COLUMN building_id INTEGER"
                 " REFERENCES buildings(id);"
                 "ALTER TABLE rooms ADD COLUMN floor INTEGER;"
                 "ALTER TABLE rooms ADD COLUMN kind TEXT;"
                 "CREATE TABLE buildings ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL, address TEXT NOT NULL);"
                 "CREATE TABLE canteens ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL,"
                 "  building_id INTEGER NOT NULL REFERENCES buildings(id),"
                 "  hours TEXT NOT NULL DEFAULT '09:00–17:00');"
                 "CREATE TABLE menu_items ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  canteen_id INTEGER NOT NULL REFERENCES canteens(id),"
                 "  name TEXT NOT NULL,"
                 "  category TEXT NOT NULL,"
                 "  is_combo INTEGER NOT NULL DEFAULT 0,"
                 "  price INTEGER NOT NULL,"
                 "  kcal INTEGER, protein REAL, fat REAL, carbs REAL);"
                 "CREATE INDEX idx_menu_items_canteen ON"
                 " menu_items(canteen_id, category);"
                 "CREATE INDEX idx_rooms_building ON"
                 " rooms(building_id, floor);");
        }
        if (withData)
        {
            // Корпуса и аудитории — строки fixtures Э6а (контрольная:
            // «1207» → Корпус Б, этаж 2).
            exec(raw,
                 "INSERT INTO buildings(id, name, address) VALUES"
                 " (1,'Корпус А','проспект Научный, д. 10 (адрес"
                 " вымышленный)'),"
                 " (2,'Корпус Б','улица Парковая, д. 4 (адрес"
                 " вымышленный)');"
                 "INSERT INTO rooms(id, name, building, capacity, features,"
                 " building_id, floor, kind) VALUES"
                 " (1,'1207','Корпус Б',24,'проектор, доска',2,2,"
                 "  'аудитория'),"
                 " (2,'3105','Корпус А',18,'доска, столы для"
                 " переговоров',1,3,'аудитория'),"
                 " (3,'Б-204','Корпус Б',60,'проектор, микрофон,"
                 " компьютеры',2,2,'аудитория');");
            // Столовые fixtures Э6б + компактное меню из контрольных строк.
            exec(raw,
                 "INSERT INTO canteens(id, name, building_id, hours) VALUES"
                 " (1,'Столовая №1',1,'09:00–18:00'),"
                 " (2,'Кофе-пойнт ФМО',2,'08:30–16:00'),"
                 // (3) — дополнение теста: столовая БЕЗ меню (путь «пустое
                 // меню существующей — 200 []», не NotFound).
                 " (3,'Буфет (вымышленный)',1,'10:00–15:00');"
                 "INSERT INTO menu_items(id, canteen_id, name, category,"
                 " is_combo, price, kcal, protein, fat, carbs) VALUES"
                 " (1,1,'Комплексный обед №1','combo',1,320,620,28.0,22.0,"
                 "  65.0),"
                 " (2,1,'Борщ со сметаной','soup',0,90,260,9.0,11.0,28.0),"
                 " (3,1,'Вода без газа 0,5 л','drink',0,50,NULL,NULL,NULL,"
                 "  NULL),"
                 " (4,2,'Капучино','drink',0,130,140,8.0,5.0,12.0);");
        }
        sqlite3_close(raw);
        // FK на каждом соединении (Э8 шаг 5, находка Э1) — те же
        // initialization_queries, что в проде (config.json); Drogon
        // 1.9.14 исполняет их на каждом новом соединении пула.
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }
};

CanteensService::Result canteensOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<CanteensService::Result> p;
    auto f = p.get_future();
    CanteensService::list(db, [&p](CanteensService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

CanteensService::MenuResult menuOf(const drogon::orm::DbClientPtr& db,
                                   int64_t canteenId)
{
    std::promise<CanteensService::MenuResult> p;
    auto f = p.get_future();
    CanteensService::menu(db, canteenId, [&p](CanteensService::MenuResult r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

MapService::Result buildingsOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<MapService::Result> p;
    auto f = p.get_future();
    MapService::buildings(db, [&p](MapService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

MapService::Result roomsOf(const drogon::orm::DbClientPtr& db,
                           MapService::RoomFilters filters = {})
{
    std::promise<MapService::Result> p;
    auto f = p.get_future();
    MapService::rooms(db, filters, [&p](MapService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

std::vector<int64_t> idsOf(const Json::Value& arr)
{
    std::vector<int64_t> out;
    for (const auto& it : arr)
        out.push_back(it["id"].as<int64_t>());
    return out;
}

}  // namespace

TEST_CASE("CanteensService::list — корпус и часы, порядок по id")
{
    Fixture fx;
    auto r = canteensOf(fx.db);
    REQUIRE(r.ok);
    const auto& arr = r.body["canteens"];
    REQUIRE(arr.size() == 3);  // 2 столовых fixtures + буфет без меню (тест)
    CHECK(arr[0]["id"].as<int64_t>() == 1);
    CHECK(arr[0]["name"].asString() == "Столовая №1");
    CHECK(arr[0]["building_id"].as<int64_t>() == 1);
    CHECK(arr[0]["building"].asString() == "Корпус А");
    CHECK(arr[0]["hours"].asString() == "09:00–18:00");
    CHECK(arr[1]["id"].as<int64_t>() == 2);
    CHECK(arr[1]["building"].asString() == "Корпус Б");
    CHECK(arr[1]["hours"].asString() == "08:30–16:00");
}

TEST_CASE("CanteensService::list — пустая таблица → 200 {canteens:[]}")
{
    Fixture fx(false);
    auto r = canteensOf(fx.db);
    REQUIRE(r.ok);
    CHECK(r.body["canteens"].size() == 0);
}

TEST_CASE("CanteensService::menu — цены, комплексные, КБЖУ (null у воды)")
{
    Fixture fx;
    auto r = menuOf(fx.db, 1);
    REQUIRE(r.status == CanteensService::MenuStatus::Ok);
    // Шапка столовой (FE без второго запроса).
    CHECK(r.body["canteen_id"].as<int64_t>() == 1);
    CHECK(r.body["canteen"].asString() == "Столовая №1");
    CHECK(r.body["building"].asString() == "Корпус А");
    CHECK(r.body["hours"].asString() == "09:00–18:00");
    const auto& arr = r.body["menu"];
    REQUIRE(arr.size() == 3);
    CHECK(arr[0]["name"].asString() == "Комплексный обед №1");
    CHECK(arr[0]["category"].asString() == "combo");
    CHECK(arr[0]["is_combo"].asBool() == true);  // «комплексные»
    CHECK(arr[0]["price"].as<int64_t>() == 320);  // ₽
    CHECK(arr[0]["kcal"].as<int64_t>() == 620);
    CHECK(arr[0]["protein"].asDouble() == doctest::Approx(28.0));
    CHECK(arr[0]["fat"].asDouble() == doctest::Approx(22.0));
    CHECK(arr[0]["carbs"].asDouble() == doctest::Approx(65.0));
    CHECK(arr[1]["is_combo"].asBool() == false);
    // КБЖУ nullable: вода без КБЖУ — null, не 0 (fixtures Э6б).
    CHECK(arr[2]["name"].asString() == "Вода без газа 0,5 л");
    CHECK(arr[2]["kcal"].isNull());
    CHECK(arr[2]["protein"].isNull());
    CHECK(arr[2]["fat"].isNull());
    CHECK(arr[2]["carbs"].isNull());
}

TEST_CASE("CanteensService::menu — несуществующая столовая → NotFound (404)")
{
    Fixture fx;
    auto r = menuOf(fx.db, 999);
    CHECK(r.status == CanteensService::MenuStatus::NotFound);
}

TEST_CASE("CanteensService::menu — пустое меню существующей — 200 []")
{
    Fixture fx;
    // Столовая 3 (дополнение теста) существует, но без позиций меню.
    auto r = menuOf(fx.db, 3);
    REQUIRE(r.status == CanteensService::MenuStatus::Ok);
    CHECK(r.body["canteen"].asString() == "Буфет (вымышленный)");
    CHECK(r.body["menu"].size() == 0);
}

TEST_CASE("CanteensService::menu — пустая таблица: любая id → NotFound")
{
    Fixture fx(false);  // схема есть, данных нет: столовых нет
    auto r = menuOf(fx.db, 1);
    CHECK(r.status == CanteensService::MenuStatus::NotFound);
}

TEST_CASE("MapService::buildings — корпуса")
{
    Fixture fx;
    auto r = buildingsOf(fx.db);
    REQUIRE(r.ok);
    const auto& arr = r.body["buildings"];
    REQUIRE(arr.size() == 2);
    CHECK(arr[0]["id"].as<int64_t>() == 1);
    CHECK(arr[0]["name"].asString() == "Корпус А");
    CHECK(arr[0]["address"].asString() ==
          "проспект Научный, д. 10 (адрес вымышленный)");
    CHECK(arr[1]["name"].asString() == "Корпус Б");
}

TEST_CASE("MapService::rooms — поиск по номеру «1207» → корпус/этаж/тип")
{
    Fixture fx;
    MapService::RoomFilters f;
    f.q = "1207";
    auto r = roomsOf(fx.db, f);
    REQUIRE(r.ok);
    const auto& arr = r.body["rooms"];
    REQUIRE(arr.size() == 1);
    CHECK(arr[0]["id"].as<int64_t>() == 1);
    CHECK(arr[0]["name"].asString() == "1207");
    CHECK(arr[0]["building_id"].as<int64_t>() == 2);
    CHECK(arr[0]["building"].asString() == "Корпус Б");
    CHECK(arr[0]["floor"].as<int64_t>() == 2);
    CHECK(arr[0]["kind"].asString() == "аудитория");
}

TEST_CASE("MapService::rooms — подстрока номера, фильтр building_id, комбо")
{
    Fixture fx;
    // Подстрока: «204» находит «Б-204».
    MapService::RoomFilters part;
    part.q = "204";
    auto r1 = roomsOf(fx.db, part);
    REQUIRE(r1.ok);
    const auto ids1 = idsOf(r1.body["rooms"]);
    REQUIRE(ids1.size() == 1);
    CHECK(ids1[0] == 3);

    // Фильтр корпуса: building_id=2 → «1207» и «Б-204».
    MapService::RoomFilters b2;
    b2.buildingId = 2;
    auto r2 = roomsOf(fx.db, b2);
    REQUIRE(r2.ok);
    const auto ids2 = idsOf(r2.body["rooms"]);
    REQUIRE(ids2.size() == 2);
    CHECK(ids2[0] == 1);
    CHECK(ids2[1] == 3);

    // Комбо-фильтр: «1207» в корпусе 1 — пусто (не ошибка).
    MapService::RoomFilters combo;
    combo.q = "1207";
    combo.buildingId = 1;
    auto r3 = roomsOf(fx.db, combo);
    REQUIRE(r3.ok);
    CHECK(r3.body["rooms"].size() == 0);

    // Без совпадений — 200 []; «%» экранирован — не «всё подряд».
    MapService::RoomFilters none;
    none.q = "нет-такого";
    auto r4 = roomsOf(fx.db, none);
    REQUIRE(r4.ok);
    CHECK(r4.body["rooms"].size() == 0);
    MapService::RoomFilters wildcard;
    wildcard.q = "%";
    auto r5 = roomsOf(fx.db, wildcard);
    REQUIRE(r5.ok);
    CHECK(r5.body["rooms"].size() == 0);
}

TEST_CASE("MapService::rooms — без фильтров: все комнаты каталога")
{
    Fixture fx;
    auto r = roomsOf(fx.db);
    REQUIRE(r.ok);
    CHECK(idsOf(r.body["rooms"]).size() == 3);
}

TEST_CASE("Canteens/Map — ошибка БД — fail-closed (Error/ok=false)")
{
    Fixture fx(false, false);  // схема не создана: выборки падают
    auto r1 = canteensOf(fx.db);
    CHECK_FALSE(r1.ok);
    auto r2 = menuOf(fx.db, 1);
    CHECK(r2.status == CanteensService::MenuStatus::Error);
    auto r3 = buildingsOf(fx.db);
    CHECK_FALSE(r3.ok);
    auto r4 = roomsOf(fx.db);
    CHECK_FALSE(r4.ok);
}
