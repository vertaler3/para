// Юнит-тесты RoomsService (Э5а шаги 1–2; 02 §5.2, миграция 0005):
//  - catalog — каталог аудиторий (поля 0005, порядок по id), пустая таблица —
//    200 {"rooms":[]}, а не ошибка;
//  - slots — сетка дня по 60 минут (08:00–22:00 UTC, 14 слотов) с тремя
//    состояниями 02 §5.2: free / booked (чужая активная бронь) / mine
//    («ваша бронь»); cancelled слот НЕ занимает; blocked (блокировка
//    сотрудника) в сетке ШАГА 1 не участвует — запрос по status='booked'
//    имплицирует условие частичного индекса idx_bookings_active
//    (IN ('booked','blocked') даёт SCAN — проверено EXPLAIN на этом шаге);
//    отображение блокировок — шаг 3 Э5а; состояние зависит от user_id
//    сессии (изоляция «моя/чужая»);
//  - create (шаг 2) — бронь интервала: Created (201), «двойная бронь» того
//    же слота → SlotTaken (400-класс), частичное пересечение → Conflict
//    (409-класс), касания границ полуинтервалов НЕ пересекаются; исключение
//    БД на INSERT (гонка UNIQUE / триггер в тесте) → SlotTaken (400), не
//    Error/500; InvalidBody (строгий ISO «…Z», ends_at > starts_at);
//  - myBookings (шаг 2) — только свои активные (booked), JOIN rooms,
//    фильтр upcoming (ends_at >= nowIso, now передаётся явно);
//  - cancelById (шаг 2) — своя booked → Canceled (204-класс), повторная →
//    NotActive (409-класс), несуществующая → NotFound; guard-цепочка DELETE:
//    чужая → Forbidden (403) — OwnershipGuard::check(Table::RoomBookings);
//  - несуществующая комната → NotFound (404 контроллером), невалидная дата
//    → Error (fail-closed, контроллер до сервиса отвечает 400);
//  - ошибка БД — fail-closed (Error → 500 контроллером), не пустая сетка.
// Данные копируют sql/fixtures.sql Э5а (3 аудитории; брони 1–2 демо-недели:
// Олег room 1 11:00, Аня room 2 14:00 06.11) + помеченные дополнения теста:
// бронь 3 (cancelled, не занимает) и блокировка 4 (blocked, dean —
// занимает). Новые брони шага 2 — на 2026-11-13 (гейт-дата: фикстуры её не
// касаются). Временная SQLite через настоящий DbClient — как в тестах Э3/Э4;
// даты/now передаются в сервис явно — wall-clock в тестах не участвует.
#include <chrono>
#include <filesystem>
#include <future>
#include <map>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/OwnershipGuard.h"
#include "services/RoomsService.h"

namespace fs = std::filesystem;
using para::OwnershipGuard;
using para::RoomsService;

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
               ("para-rooms-" + std::to_string(stamp) + "-" +
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

// Схема — колонки rooms/room_bookings из миграции 0005 (DDL 02 §4.1)
// + users (FK room_bookings.user_id). CHECK и частичный UNIQUE
// idx_bookings_active включены — контракт «двойная бронь невозможна на
// уровне БД» соседствует с сеткой (вставки теста ему подчиняются).
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withRooms = true, bool withTables = true)
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
                 "INSERT INTO users VALUES (1,'anya.demo','student'),"
                 " (2,'oleg.demo','student'), (3,'mark.demo','student'),"
                 " (5,'dean.demo','staff');"
                 "CREATE TABLE rooms ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  name TEXT NOT NULL,"
                 "  building TEXT NOT NULL,"
                 "  capacity INTEGER NOT NULL,"
                 "  features TEXT NOT NULL DEFAULT '');"
                 "CREATE TABLE room_bookings ("
                 "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "  room_id INTEGER NOT NULL REFERENCES rooms(id),"
                 "  user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE"
                 " CASCADE,"
                 "  starts_at TEXT NOT NULL,"
                 "  ends_at TEXT NOT NULL,"
                 "  status TEXT NOT NULL DEFAULT 'booked'"
                 "    CHECK (status IN ('booked','cancelled','blocked')),"
                 "  created_at TEXT NOT NULL DEFAULT"
                 " (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
                 "CREATE UNIQUE INDEX idx_bookings_active ON"
                 " room_bookings(room_id, starts_at) WHERE status ="
                 " 'booked';"
                 "CREATE INDEX idx_bookings_user ON room_bookings(user_id,"
                 " starts_at);");
            if (withRooms)
            {
                // 3 аудитории каталога — точные строки sql/fixtures.sql Э5а.
                exec(raw,
                     "INSERT INTO rooms(id, name, building, capacity,"
                     " features) VALUES"
                     " (1,'1207','Корпус А',24,'проектор, доска'),"
                     " (2,'3105','Корпус А',18,'доска, столы для"
                     " переговоров'),"
                     " (3,'Б-204','Корпус Б',60,'проектор, микрофон,"
                     " компьютеры');");
                // Брони фикстур (1–2, демо-неделя) + помеченные дополнения
                // теста (3 — cancelled, 4 — blocked, их в fixtures.sql нет).
                exec(raw,
                     "INSERT INTO room_bookings(id, room_id, user_id,"
                     " starts_at, ends_at, status, created_at) VALUES"
                     " (1,1,2,'2026-11-06T11:00:00Z','2026-11-06T12:00:00Z',"
                     "  'booked','2026-11-05T19:00:00Z'),"
                     " (2,2,1,'2026-11-06T14:00:00Z','2026-11-06T15:00:00Z',"
                     "  'booked','2026-11-05T19:10:00Z'),"
                     " (3,1,3,'2026-11-06T09:00:00Z','2026-11-06T10:00:00Z',"
                     "  'cancelled','2026-11-05T19:20:00Z'),"
                     " (4,3,5,'2026-11-06T10:00:00Z','2026-11-06T11:00:00Z',"
                     "  'blocked','2026-11-05T19:30:00Z');");
            }
        }
        sqlite3_close(raw);
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }
};

RoomsService::Result catalogOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<RoomsService::Result> p;
    auto f = p.get_future();
    RoomsService::catalog(db, [&p](RoomsService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

RoomsService::SlotsResult slotsOf(const drogon::orm::DbClientPtr& db,
                                  int64_t roomId,
                                  int64_t userId,
                                  const std::string& date)
{
    std::promise<RoomsService::SlotsResult> p;
    auto f = p.get_future();
    RoomsService::slots(db,
                        roomId,
                        userId,
                        date,
                        [&p](RoomsService::SlotsResult r) {
                            p.set_value(std::move(r));
                        });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

RoomsService::Lookup lookupOf(const drogon::orm::DbClientPtr& db,
                              int64_t roomId)
{
    std::promise<RoomsService::Lookup> p;
    auto f = p.get_future();
    RoomsService::roomById(db, roomId, [&p](auto lookup, auto) {
        p.set_value(lookup);
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// Карта «starts_at → state» сетки.
std::map<std::string, std::string> statesOf(const Json::Value& body)
{
    std::map<std::string, std::string> out;
    for (const auto& s : body["slots"])
        out[s["starts_at"].asString()] = s["state"].asString();
    return out;
}

std::size_t countState(const Json::Value& body, const std::string& state)
{
    std::size_t n = 0;
    for (const auto& s : body["slots"])
        if (s["state"].asString() == state)
            ++n;
    return n;
}

// --- Шаг 2: брони ---------------------------------------------------------
RoomsService::CreateResult createOf(const drogon::orm::DbClientPtr& db,
                                    int64_t roomId,
                                    int64_t userId,
                                    const std::string& startsAt,
                                    const std::string& endsAt)
{
    std::promise<RoomsService::CreateResult> p;
    auto f = p.get_future();
    RoomsService::create(db,
                         roomId,
                         userId,
                         startsAt,
                         endsAt,
                         [&p](RoomsService::CreateResult r) {
                             p.set_value(std::move(r));
                         });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

RoomsService::Result myBookingsOf(const drogon::orm::DbClientPtr& db,
                                  int64_t userId,
                                  bool upcoming,
                                  const std::string& nowIso)
{
    std::promise<RoomsService::Result> p;
    auto f = p.get_future();
    RoomsService::myBookings(db,
                             userId,
                             upcoming,
                             nowIso,
                             [&p](RoomsService::Result r) {
                                 p.set_value(std::move(r));
                             });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

RoomsService::CancelStatus cancelOf(const drogon::orm::DbClientPtr& db,
                                    int64_t bookingId)
{
    std::promise<RoomsService::CancelStatus> p;
    auto f = p.get_future();
    RoomsService::cancelById(db, bookingId, [&p](RoomsService::CancelStatus s) {
        p.set_value(s);
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

OwnershipGuard::Verdict guardOf(const drogon::orm::DbClientPtr& db,
                                int64_t sessionUserId,
                                int64_t bookingId)
{
    // Ровно тот вызов, что делает RoomsCtrl::cancelBooking до любых данных.
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db,
                          OwnershipGuard::Table::RoomBookings,
                          sessionUserId,
                          bookingId,
                          [&p](OwnershipGuard::Verdict v) { p.set_value(v); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

}  // namespace

TEST_CASE("RoomsService: validDate — формат и календарность")
{
    CHECK(RoomsService::validDate("2026-11-06"));
    CHECK(RoomsService::validDate("2024-02-29"));   // високосный
    CHECK_FALSE(RoomsService::validDate("2026-02-29"));   // не високосный
    CHECK_FALSE(RoomsService::validDate("2026-02-30"));   // timegm нормализует
    CHECK_FALSE(RoomsService::validDate("2026-13-01"));
    CHECK_FALSE(RoomsService::validDate("2026-00-10"));
    CHECK_FALSE(RoomsService::validDate("abc"));
    CHECK_FALSE(RoomsService::validDate("2026-11-06T10:00:00Z"));  // не дата
    CHECK_FALSE(RoomsService::validDate(""));
    CHECK(RoomsService::todayUtc().size() == 10);  // YYYY-MM-DD
}

TEST_CASE("RoomsService: catalog — 3 аудитории фикстур, порядок по id")
{
    Fixture f;
    const auto r = catalogOf(f.db);
    REQUIRE(r.ok);
    const auto& rooms = r.body["rooms"];
    REQUIRE(rooms.size() == 3);
    CHECK(rooms[0]["id"].as<int64_t>() == 1);
    CHECK(rooms[0]["name"].asString() == "1207");
    CHECK(rooms[0]["building"].asString() == "Корпус А");
    CHECK(rooms[0]["capacity"].as<int64_t>() == 24);
    CHECK(rooms[0]["features"].asString() == "проектор, доска");
    CHECK(rooms[1]["name"].asString() == "3105");
    CHECK(rooms[2]["name"].asString() == "Б-204");
    CHECK(rooms[2]["building"].asString() == "Корпус Б");
    CHECK(rooms[2]["capacity"].as<int64_t>() == 60);
}

TEST_CASE("RoomsService: catalog — пустая таблица, 200 с пустым rooms")
{
    Fixture f(/*withRooms=*/false);
    const auto r = catalogOf(f.db);
    REQUIRE(r.ok);
    CHECK(r.body["rooms"].size() == 0);
}

TEST_CASE("RoomsService: slots — сетка 60 минут, три состояния 02 §5.2")
{
    Fixture f;
    // Аня (user 1), пятница 06.11 демо-недели, ауд. 1207 (room 1):
    // бронь Олега 11:00 → «занято», отменённая бронь Марка 09:00 — слот
    // свободен, остальные свободны.
    const auto r = slotsOf(f.db, 1, 1, "2026-11-06");
    REQUIRE(r.status == RoomsService::SlotsStatus::Ok);
    CHECK(r.body["room_id"].as<int64_t>() == 1);
    CHECK(r.body["room"].asString() == "1207");
    CHECK(r.body["building"].asString() == "Корпус А");
    CHECK(r.body["date"].asString() == "2026-11-06");
    REQUIRE(r.body["slots"].size() == 14);  // 08:00–22:00 по 60 минут

    const auto states = statesOf(r.body);
    CHECK(states.begin()->first == "2026-11-06T08:00:00Z");
    CHECK(states.rbegin()->first == "2026-11-06T21:00:00Z");
    CHECK(states.at("2026-11-06T09:00:00Z") == "free");   // cancelled не занял
    CHECK(states.at("2026-11-06T11:00:00Z") == "booked"); // чужая бронь Олега
    CHECK(countState(r.body, "booked") == 1);
    CHECK(countState(r.body, "mine") == 0);
    CHECK(countState(r.body, "free") == 13);
    // Форма слота: starts_at/ends_at/state (marker времени — starts_at).
    const auto& first = r.body["slots"][0];
    CHECK(first["starts_at"].asString() == "2026-11-06T08:00:00Z");
    CHECK(first["ends_at"].asString() == "2026-11-06T09:00:00Z");
    CHECK(first["state"].asString() == "free");

    // «Ваша бронь»: та же сетка для Ани в ауд. 3105 — её бронь 14:00.
    const auto mine = slotsOf(f.db, 2, 1, "2026-11-06");
    REQUIRE(mine.status == RoomsService::SlotsStatus::Ok);
    CHECK(mine.body["slots"][6]["starts_at"].asString() ==
          "2026-11-06T14:00:00Z");  // индекс 6 = 14:00 (8+6)
    CHECK(mine.body["slots"][6]["state"].asString() == "mine");
    CHECK(countState(mine.body, "mine") == 1);
    CHECK(countState(mine.body, "free") == 13);

    // Изоляция: для Марка та же бронь Ани — «занято», не «ваша бронь».
    const auto mark = slotsOf(f.db, 2, 3, "2026-11-06");
    REQUIRE(mark.status == RoomsService::SlotsStatus::Ok);
    CHECK(mark.body["slots"][6]["state"].asString() == "booked");

    // Блокировка сотрудника (dean, room 3 10:00, status='blocked') в сетке
    // шага 1 НЕ участвует — контракт «состояние по активным броням
    // status='booked'» (см. шапку RoomsService.h; отображение блокировок —
    // шаг 3 Э5а). Слот 10:00 остаётся «free» для всех.
    const auto stud = slotsOf(f.db, 3, 1, "2026-11-06");
    REQUIRE(stud.status == RoomsService::SlotsStatus::Ok);
    CHECK(stud.body["slots"][2]["state"].asString() == "free");  // 10:00
    CHECK(countState(stud.body, "free") == 14);
}

TEST_CASE("RoomsService: slots — другая дата и гейт-дата 2026-11-13 свободны")
{
    Fixture f;
    // Понедельник демо-недели: броней нет — вся сетка свободна.
    const auto mon = slotsOf(f.db, 1, 1, "2026-11-02");
    REQUIRE(mon.status == RoomsService::SlotsStatus::Ok);
    CHECK(countState(mon.body, "free") == 14);

    // Дата приёмочного гейта Э5а: фикстуры её не касаются — все слоты
    // всех аудиторий свободны (бронирование шага 2 пройдёт без конфликта).
    for (int64_t room = 1; room <= 3; ++room)
    {
        const auto r = slotsOf(f.db, room, 1, "2026-11-13");
        REQUIRE(r.status == RoomsService::SlotsStatus::Ok);
        CHECK(r.body["date"].asString() == "2026-11-13");
        CHECK(countState(r.body, "free") == 14);
        CHECK(countState(r.body, "booked") == 0);
        CHECK(countState(r.body, "mine") == 0);
    }
}

TEST_CASE("RoomsService: slots — несуществующая комната и невалидная дата")
{
    Fixture f;
    // Комнаты нет → NotFound (контроллер отвечает 404 not_found).
    CHECK(slotsOf(f.db, 999999, 1, "2026-11-06").status ==
          RoomsService::SlotsStatus::NotFound);
    CHECK(lookupOf(f.db, 999999) == RoomsService::Lookup::NotFound);
    CHECK(lookupOf(f.db, 1) == RoomsService::Lookup::Found);

    // Невалидная дата — Error (fail-closed; контроллер отвечает 400 ДО
    // сервиса — здесь страховка от «весь день свободен»).
    CHECK(slotsOf(f.db, 1, 1, "2026-02-30").status ==
          RoomsService::SlotsStatus::Error);
    CHECK(slotsOf(f.db, 1, 1, "не-дата").status ==
          RoomsService::SlotsStatus::Error);
}

TEST_CASE("RoomsService: ошибка БД — fail-closed (Error → 500)")
{
    // Схема не создана: выборки падают — сервис отвечает Error (500
    // контроллером, формат 02 §5.1), а не пустым каталогом/сеткой.
    Fixture f(/*withRooms=*/false, /*withTables=*/false);
    CHECK_FALSE(catalogOf(f.db).ok);
    CHECK(slotsOf(f.db, 1, 1, "2026-11-06").status ==
          RoomsService::SlotsStatus::Error);
    CHECK(lookupOf(f.db, 1) == RoomsService::Lookup::Error);
}

TEST_CASE("RoomsService: create — бронь слота, сетка и «мои брони» обновляются")
{
    Fixture f;
    // Бронь гейт-даты 13.11, слот 10:00–11:00 UTC (Аня).
    const auto r = createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                            "2026-11-13T11:00:00Z");
    REQUIRE(r.status == RoomsService::CreateStatus::Created);
    CHECK(r.body["id"].as<int64_t>() > 0);
    CHECK(r.body["room_id"].as<int64_t>() == 1);
    CHECK(r.body["starts_at"].asString() == "2026-11-13T10:00:00Z");
    CHECK(r.body["ends_at"].asString() == "2026-11-13T11:00:00Z");
    CHECK(r.body["status"].asString() == "booked");

    // Сетка того дня показывает бронь: для Ани — mine, для Олега — booked.
    const auto gridAnya = slotsOf(f.db, 1, 1, "2026-11-13");
    REQUIRE(gridAnya.status == RoomsService::SlotsStatus::Ok);
    CHECK(gridAnya.body["slots"][2]["state"].asString() == "mine");   // 10:00
    CHECK(countState(gridAnya.body, "free") == 13);
    const auto gridOleg = slotsOf(f.db, 1, 2, "2026-11-13");
    REQUIRE(gridOleg.status == RoomsService::SlotsStatus::Ok);
    CHECK(gridOleg.body["slots"][2]["state"].asString() == "booked");

    // «Мои брони»: JOIN rooms (имя/корпус); у Ани кроме новой — активная
    // фикстурная бронь id 2 (06.11 14:00, room 2) → всего 2.
    const auto mine = myBookingsOf(f.db, 1, false, "2026-11-13T09:00:00Z");
    REQUIRE(mine.ok);
    REQUIRE(mine.body["bookings"].size() == 2);
    bool found13 = false;
    for (const auto& b : mine.body["bookings"])
    {
        if (b["starts_at"].asString() == "2026-11-13T10:00:00Z")
        {
            found13 = true;
            CHECK(b["room"].asString() == "1207");
            CHECK(b["building"].asString() == "Корпус А");
            CHECK(b["status"].asString() == "booked");
        }
    }
    CHECK(found13);
    // Изоляция: у Олега только его активная фикстурная бронь id 1
    // (06.11 11:00); брони Ани не видны.
    const auto oleg = myBookingsOf(f.db, 2, false, "2026-11-13T09:00:00Z");
    REQUIRE(oleg.ok);
    REQUIRE(oleg.body["bookings"].size() == 1);
    CHECK(oleg.body["bookings"][0]["starts_at"].asString() ==
          "2026-11-06T11:00:00Z");
}

TEST_CASE("RoomsService: create — дубль слота 400-класс, пересечение 409-класс")
{
    Fixture f;
    REQUIRE(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                     "2026-11-13T11:00:00Z").status ==
            RoomsService::CreateStatus::Created);

    // Тот же слот другим студентом → SlotTaken (400 «двойная бронь»,
    // гайд BE §8 «повторная бронь того же слота другим клиентом → 400»).
    CHECK(createOf(f.db, 1, 2, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::SlotTaken);
    // Тот же слот тем же студентом — тоже двойная бронь.
    CHECK(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::SlotTaken);
    // Другая комната — тот же слот свободен (пересечение по комнате).
    CHECK(createOf(f.db, 2, 2, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::Created);

    // Частичное перекрытие (10:30–11:30 против 10:00–11:00) → Conflict
    // (409 «конфликт, только что заняли», случай FE-сетки 03 §4.9.5).
    CHECK(createOf(f.db, 1, 2, "2026-11-13T10:30:00Z",
                   "2026-11-13T11:30:00Z").status ==
          RoomsService::CreateStatus::Conflict);
    // Пересечение И «тот же starts_at» — SlotTaken (это и ловит UNIQUE).
    CHECK(createOf(f.db, 1, 2, "2026-11-13T10:00:00Z",
                   "2026-11-13T12:00:00Z").status ==
          RoomsService::CreateStatus::SlotTaken);

    // Касания границ полуинтервалов НЕ пересекаются: [11:00,12:00) и
    // [09:00,10:00) против [10:00,11:00) — обе бронируются.
    CHECK(createOf(f.db, 1, 2, "2026-11-13T11:00:00Z",
                   "2026-11-13T12:00:00Z").status ==
          RoomsService::CreateStatus::Created);
    CHECK(createOf(f.db, 1, 2, "2026-11-13T09:00:00Z",
                   "2026-11-13T10:00:00Z").status ==
          RoomsService::CreateStatus::Created);

    // В соседней комнате пересечение 10:30–11:30 не конфликтует ни с чем.
    CHECK(createOf(f.db, 3, 2, "2026-11-13T10:30:00Z",
                   "2026-11-13T11:30:00Z").status ==
          RoomsService::CreateStatus::Created);
}

TEST_CASE("RoomsService: create — InvalidBody, NotFound, fail-closed")
{
    Fixture f;
    // Не ISO «…Z» / интервал не валиден → InvalidBody (400 bad_request).
    CHECK(createOf(f.db, 1, 1, "2026-11-13T10:00:00",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::InvalidBody);  // нет Z
    CHECK(createOf(f.db, 1, 1, "2026-11-13 10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::InvalidBody);  // пробел вместо T
    CHECK(createOf(f.db, 1, 1, "2026-11-13T25:00:00Z",
                   "2026-11-13T26:00:00Z").status ==
          RoomsService::CreateStatus::InvalidBody);  // часы > 23
    CHECK(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                   "2026-11-13T10:00:00Z").status ==
          RoomsService::CreateStatus::InvalidBody);  // нулевой интервал
    CHECK(createOf(f.db, 1, 1, "2026-11-13T11:00:00Z",
                   "2026-11-13T10:00:00Z").status ==
          RoomsService::CreateStatus::InvalidBody);  // конц раньше начала
    CHECK(createOf(f.db, 1, 1, "abc", "def").status ==
          RoomsService::CreateStatus::InvalidBody);

    // Комнаты нет → NotFound (404 контроллером).
    CHECK(createOf(f.db, 999999, 1, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::NotFound);
}

TEST_CASE("RoomsService: create — исключение БД на INSERT → SlotTaken (400)")
{
    // Ветка гонки UNIQUE (серверная проверка обеих параллельных POST прошла
    // до INSERT'ов): в тесте воспроизводится триггером ABORT на INSERT —
    // серверная проверка молчит (броней нет), INSERT падает — сервис
    // обязан отдать SlotTaken (400 «двойная бронь»), НЕ Error/500
    // (постановка шага 2: «поймай исключение БД и отдай 400, а не 500»).
    Fixture f;
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(f.dbPath.c_str(),
                            &raw,
                            SQLITE_OPEN_READWRITE,
                            nullptr) == SQLITE_OK);
    exec(raw,
         "CREATE TRIGGER deny_booking BEFORE INSERT ON room_bookings "
         "BEGIN SELECT RAISE(ABORT, 'test: deny'); END;");
    sqlite3_close(raw);

    CHECK(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::SlotTaken);
}

TEST_CASE("RoomsService: myBookings — booked only и фильтр upcoming")
{
    Fixture f;
    REQUIRE(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                     "2026-11-13T11:00:00Z").status ==
            RoomsService::CreateStatus::Created);   // id 5, предстоящая
    REQUIRE(createOf(f.db, 2, 1, "2026-11-13T08:00:00Z",
                     "2026-11-13T09:00:00Z").status ==
            RoomsService::CreateStatus::Created);   // id 6, уже прошла
    // Отменённая (fixtures id 3, user 3) в «моих брони» не возвращается.
    const auto markAll = myBookingsOf(f.db, 3, false, "2026-11-13T09:30:00Z");
    REQUIRE(markAll.ok);
    CHECK(markAll.body["bookings"].size() == 0);

    // У Ани: фикстурная id 2 (06.11 14:00) + созданные id 6 (13.11 08:00)
    // и id 5 (13.11 10:00) — три активных, по starts_at.
    const auto all = myBookingsOf(f.db, 1, false, "2026-11-13T09:30:00Z");
    REQUIRE(all.ok);
    REQUIRE(all.body["bookings"].size() == 3);
    CHECK(all.body["bookings"][0]["starts_at"].asString() ==
          "2026-11-06T14:00:00Z");
    CHECK(all.body["bookings"][1]["starts_at"].asString() ==
          "2026-11-13T08:00:00Z");  // ORDER BY starts_at
    CHECK(all.body["bookings"][2]["starts_at"].asString() ==
          "2026-11-13T10:00:00Z");

    // upcoming (?upcoming=1): только предстоящие (ends_at >= now).
    // Фикстурная (ends 06.11T15:00) и прошедшая 08:00–09:00 — вне.
    const auto up = myBookingsOf(f.db, 1, true, "2026-11-13T09:30:00Z");
    REQUIRE(up.ok);
    REQUIRE(up.body["bookings"].size() == 1);
    CHECK(up.body["bookings"][0]["starts_at"].asString() ==
          "2026-11-13T10:00:00Z");

    // Отмена убирает бронь из списка (активные — только booked).
    REQUIRE(cancelOf(f.db, 5) == RoomsService::CancelStatus::Canceled);
    const auto after = myBookingsOf(f.db, 1, false, "2026-11-13T09:30:00Z");
    REQUIRE(after.ok);
    REQUIRE(after.body["bookings"].size() == 2);
    CHECK(after.body["bookings"][0]["starts_at"].asString() ==
          "2026-11-06T14:00:00Z");
    CHECK(after.body["bookings"][1]["starts_at"].asString() ==
          "2026-11-13T08:00:00Z");
}

TEST_CASE("RoomsService: cancelById и guard-цепочка DELETE (своя/чужая)")
{
    Fixture f;
    // Своя активная брони фикстур: id 1 — Олег (room 1, 06.11 11:00),
    // id 2 — Аня (room 2, 06.11 14:00).
    // Чужая → Forbidden через OwnershipGuard::check(Table::RoomBookings)
    // (правило код-ревью; шаг 8 автотеста 02 §9.3).
    CHECK(guardOf(f.db, 1, 1) == OwnershipGuard::Verdict::Forbidden);
    CHECK(guardOf(f.db, 2, 2) == OwnershipGuard::Verdict::Forbidden);
    CHECK(guardOf(f.db, 2, 1) == OwnershipGuard::Verdict::Own);
    CHECK(guardOf(f.db, 1, 999999) == OwnershipGuard::Verdict::NotFound);

    // Своя → Canceled; сетка освобождается (cancelled слот не занимает).
    REQUIRE(cancelOf(f.db, 1) == RoomsService::CancelStatus::Canceled);
    const auto grid = slotsOf(f.db, 1, 2, "2026-11-06");
    REQUIRE(grid.status == RoomsService::SlotsStatus::Ok);
    CHECK(grid.body["slots"][3]["state"].asString() == "free");  // 11:00
    // Повторная отмена → NotActive (409, статус не booked).
    CHECK(cancelOf(f.db, 1) == RoomsService::CancelStatus::NotActive);
    // Несуществующая → NotFound (после Own — гонка; не маскируется).
    CHECK(cancelOf(f.db, 999999) == RoomsService::CancelStatus::NotFound);
    // Отмена заблокированного слота (id 4, dean, blocked) — не booked.
    CHECK(cancelOf(f.db, 4) == RoomsService::CancelStatus::NotActive);
}

TEST_CASE("RoomsService: брони — ошибка БД fail-closed (Error → 500)")
{
    Fixture f(/*withRooms=*/false, /*withTables=*/false);
    CHECK(createOf(f.db, 1, 1, "2026-11-13T10:00:00Z",
                   "2026-11-13T11:00:00Z").status ==
          RoomsService::CreateStatus::Error);
    CHECK_FALSE(myBookingsOf(f.db, 1, false, "2026-11-13T09:00:00Z").ok);
    CHECK(cancelOf(f.db, 1) == RoomsService::CancelStatus::Error);
}
