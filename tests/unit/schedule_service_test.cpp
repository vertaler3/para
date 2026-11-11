// Юнит-тесты ScheduleService/ScheduleCache (Э3 шаг 1; 02 §5.2, миграция 0002):
//  - today выбирает только свой день и только свою группу;
//  - week отдаёт неделю группы и смещается по offset (±);
//  - changes фильтрует по статусам moved/cancelled/substituted и окну days,
//    отдаёт prev_* («было → стало»);
//  - ScheduleCache: put/get, отдача закэшированной недели до инвалидации,
//    invalidate(group_id) — хук Э6;
//  - ошибка БД — fail-closed (ok=false → 500 контроллером), не пустой ответ.
// Данные копируют демо-неделю 2–8.11.2026 группы 1-ФМО из sql/fixtures.sql
// (ids 1–10: два moved 04.11, cancelled 05.11 id 7) + помеченные дополнения:
// id 11 — substituted 01.11 (прошлая неделя, окно changes), id 20 — событие
// группы 2 (изоляция групп), id 30 — вставляется в тесте кэша.
// Фикстуры v2 (10.11.2026) продолжили расписание ids 11–42 (будни 09–30.11,
// группа 1: moved 15/23/37/41, cancelled 17/30) — события v2 в тестовую БД
// НЕ копируются; номера 11/20/30 заняты дополнениями теста только в её
// собственной временной БД (в fixtures.sql это другие строки). Демо-неделя
// 1–10 в v2 байт-в-байт как была — все проверки ниже на ней.
// Временная SQLite через настоящий DbClient — как в тестах Э1/Э2; даты
// передаются в сервис явно (today="2026-11-05") — wall-clock в тестах не
// участвует. Тикер CacheMap не запускается (loop не loop()) — проверяется
// явная инвалидация, не истечение TTL; CacheMap::put по живому ключу —
// no-op до истечения TTL (семантика insert std::map) — это и проверяется.
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/ScheduleCache.h"
#include "services/ScheduleService.h"

namespace fs = std::filesystem;
using para::ScheduleCache;
using para::ScheduleService;
using para::WeekModel;

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
               ("para-schedule-" + std::to_string(stamp) + "-" +
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

// Схема — колонки таблиц groups/disciplines/profiles/schedule_events из
// миграций 0001/0002, БЕЗ FK (FK-ссылки тест не проверяет — это работа
// миграций) и без индексов полноты CHECK.
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withTables = true)
    {
        if (withTables)
            createSchema();
        // FK на каждом соединении (Э8 шаг 5, находка Э1) — те же
        // initialization_queries, что в проде (config.json); Drogon
        // 1.9.14 исполняет их на каждом новом соединении пула.
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }

    void createSchema()
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        exec(raw,
             "CREATE TABLE groups (id INTEGER PRIMARY KEY, name TEXT NOT NULL,"
             " course INTEGER NOT NULL);"
             "INSERT INTO groups VALUES (1,'1-ФМО',1),(2,'4-МЭО',4);"
             "CREATE TABLE disciplines (id INTEGER PRIMARY KEY, code TEXT NOT"
             " NULL, title TEXT NOT NULL, control_type TEXT NOT NULL);"
             "INSERT INTO disciplines VALUES"
             " (1,'ME-101','Мировая экономика','exam'),"
             " (2,'PH-101','Философия','exam'),"
             " (3,'HME-101','История МЭО','exam'),"
             " (4,'HR-101','История России','graded_credit'),"
             " (5,'FL-101','Иностранный язык','credit'),"
             " (6,'MT-401','Международная торговля','graded_credit');"
             "CREATE TABLE profiles (user_id INTEGER PRIMARY KEY, group_id"
             " INTEGER NOT NULL, fio TEXT);"
             "INSERT INTO profiles VALUES (1,1,'Соколова Анна Игоревна'),"
             " (3,2,'Ливанов Марк Сергеевич');"
             // user 5 (dean.demo, staff) — профиля нет, как в фикстурах Р-6.
             "CREATE TABLE schedule_events ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  group_id INTEGER NOT NULL,"
             "  discipline_id INTEGER NOT NULL,"
             "  teacher_id INTEGER,"
             "  room TEXT NOT NULL,"
             "  kind TEXT NOT NULL,"
             "  starts_at TEXT NOT NULL,"
             "  ends_at TEXT NOT NULL,"
             "  status TEXT NOT NULL DEFAULT 'normal',"
             "  prev_room TEXT,"
             "  prev_starts_at TEXT,"
             "  prev_teacher_id INTEGER,"
             "  changed_by INTEGER,"
             "  changed_at TEXT);"
             "CREATE INDEX idx_schedule_group_time ON"
             " schedule_events(group_id, starts_at);"
             "CREATE INDEX idx_schedule_group_status ON"
             " schedule_events(group_id, status, starts_at);");
        // Демо-неделя 2–8.11.2026 группы 1 (1-ФМО) — точные строки
        // sql/fixtures.sql (ids 1–10).
        exec(raw,
             "INSERT INTO schedule_events(id, group_id, discipline_id,"
             " teacher_id, room, kind, starts_at, ends_at, status, prev_room,"
             " prev_starts_at, prev_teacher_id, changed_by, changed_at)"
             " VALUES"
             " (1,1,4,4,'1201','lecture','2026-11-02T10:00:00Z',"
             "  '2026-11-02T11:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (2,1,5,5,'2210','seminar','2026-11-02T14:00:00Z',"
             "  '2026-11-02T15:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (3,1,1,1,'3024','seminar','2026-11-03T10:00:00Z',"
             "  '2026-11-03T11:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (4,1,2,2,'3105','seminar','2026-11-03T14:00:00Z',"
             "  '2026-11-03T15:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (5,1,1,1,'4210','lecture','2026-11-04T10:00:00Z',"
             "  '2026-11-04T11:30:00Z','moved','3024',"
             "  '2026-11-04T10:00:00Z',NULL,5,'2026-11-02T15:40:00Z'),"
             " (6,1,2,2,'4210','lecture','2026-11-04T14:00:00Z',"
             "  '2026-11-04T15:30:00Z','moved','3105',"
             "  '2026-11-04T14:00:00Z',NULL,5,'2026-11-02T15:41:00Z'),"
             " (7,1,3,3,'2109','lecture','2026-11-05T10:00:00Z',"
             "  '2026-11-05T11:30:00Z','cancelled','2109',"
             "  '2026-11-05T10:00:00Z',NULL,5,'2026-11-02T15:45:00Z'),"
             " (8,1,4,4,'1201','seminar','2026-11-05T14:00:00Z',"
             "  '2026-11-05T15:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (9,1,2,2,'3105','seminar','2026-11-06T10:00:00Z',"
             "  '2026-11-06T11:30:00Z','normal',NULL,NULL,NULL,NULL,NULL),"
             " (10,1,3,3,'2109','seminar','2026-11-06T14:00:00Z',"
             "  '2026-11-06T15:30:00Z','normal',NULL,NULL,NULL,NULL,NULL);");
        // Дополнения теста — только во временной БД теста (в фикстурах v2
        // номера 11 и 20 заняты ноябрьскими событиями 09–30.11 с другими
        // данными, сюда они не копируются): замещение в прошлую
        // неделю (01.11, окно changes) и событие группы 2 на тот же день
        // (изоляция «только своя группа»).
        exec(raw,
             "INSERT INTO schedule_events(id, group_id, discipline_id,"
             " teacher_id, room, kind, starts_at, ends_at, status, prev_room,"
             " prev_starts_at, prev_teacher_id, changed_by, changed_at)"
             " VALUES"
             " (11,1,1,1,'4205','lecture','2026-11-01T10:00:00Z',"
             "  '2026-11-01T11:30:00Z','substituted','3201',"
             "  '2026-11-01T10:00:00Z',6,5,'2026-10-30T12:00:00Z'),"
             " (20,2,6,6,'5102','lecture','2026-11-05T12:10:00Z',"
             "  '2026-11-05T13:40:00Z','normal',NULL,NULL,NULL,NULL,NULL);");
        sqlite3_close(raw);
    }
};

ScheduleService::Result todayOf(const drogon::orm::DbClientPtr& db,
                                int64_t userId,
                                const std::string& today)
{
    std::promise<ScheduleService::Result> p;
    auto f = p.get_future();
    ScheduleService::today(db,
                           userId,
                           today,
                           [&p](ScheduleService::Result r) {
                               p.set_value(std::move(r));
                           });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ScheduleService::Result weekOf(const drogon::orm::DbClientPtr& db,
                               int64_t userId,
                               int offset,
                               ScheduleCache* cache,
                               const std::string& today)
{
    std::promise<ScheduleService::Result> p;
    auto f = p.get_future();
    ScheduleService::week(db,
                          userId,
                          offset,
                          cache,
                          today,
                          [&p](ScheduleService::Result r) {
                              p.set_value(std::move(r));
                          });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ScheduleService::Result changesOf(const drogon::orm::DbClientPtr& db,
                                  int64_t userId,
                                  int days,
                                  const std::string& today)
{
    std::promise<ScheduleService::Result> p;
    auto f = p.get_future();
    ScheduleService::changeList(db,
                                userId,
                                days,
                                today,
                                [&p](ScheduleService::Result r) {
                                    p.set_value(std::move(r));
                                });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

std::set<int64_t> idsOf(const Json::Value& arr)
{
    std::set<int64_t> out;
    for (const auto& e : arr)
        out.insert(e["id"].as<int64_t>());
    return out;
}

// Прямая вставка события мимо сервиса — для проверки кэша (данные меняются
// под живой записью кэша; инвалидация — контракт Э6).
void insertRawEvent(const std::string& dbPath,
                    int64_t id,
                    const std::string& startsAt,
                    const std::string& endsAt)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                            &raw,
                            SQLITE_OPEN_READWRITE,
                            nullptr) == SQLITE_OK);
    char* err = nullptr;
    const std::string sql =
        "INSERT INTO schedule_events(id, group_id, discipline_id, teacher_id,"
        " room, kind, starts_at, ends_at, status)"
        " VALUES (" + std::to_string(id) +
        ",1,5,5,'2210','seminar','" + startsAt + "','" + endsAt +
        "','normal');";
    REQUIRE(sqlite3_exec(raw, sql.c_str(), nullptr, nullptr, &err) ==
            SQLITE_OK);
    if (err != nullptr)
    {
        FAIL(err);
        sqlite3_free(err);
    }
    sqlite3_close(raw);
}

}  // namespace

TEST_CASE("ScheduleService: арифметика дат (weekStart/addDays)")
{
    // Демо-неделя фикстур: 02.11.2026 — понедельник, 05.11 — четверг,
    // 01.11 — воскресенье (проверено date-фактами фикстур).
    CHECK(ScheduleService::weekStart("2026-11-05", 0) == "2026-11-02");
    CHECK(ScheduleService::weekStart("2026-11-05", 1) == "2026-11-09");
    CHECK(ScheduleService::weekStart("2026-11-05", -1) == "2026-10-26");
    CHECK(ScheduleService::weekStart("2026-11-02", 0) == "2026-11-02");
    CHECK(ScheduleService::weekStart("2026-11-01", 0) == "2026-10-26");
    CHECK(ScheduleService::weekStart("2026-11-08", 0) == "2026-11-02");
    CHECK(ScheduleService::addDays("2026-11-05", -6) == "2026-10-30");
    CHECK(ScheduleService::addDays("2026-11-05", 1) == "2026-11-06");
    CHECK(ScheduleService::addDays("2026-10-31", 1) == "2026-11-01");
    CHECK(ScheduleService::addDays("2026-12-31", 1) == "2027-01-01");
    CHECK(ScheduleService::todayUtc().size() == 10);  // YYYY-MM-DD
}

TEST_CASE("ScheduleService: groupOfUser — студент, staff, ошибка")
{
    Fixture f;
    std::promise<std::pair<ScheduleService::GroupLookup, std::optional<int64_t>>>
        p;
    auto f1 = p.get_future();
    ScheduleService::groupOfUser(f.db, 1, [&p](auto lookup, auto groupId) {
        p.set_value({lookup, groupId});
    });
    const auto student = f1.get();
    CHECK(student.first == ScheduleService::GroupLookup::Found);
    REQUIRE(student.second.has_value());
    CHECK(*student.second == 1);  // группа 1-ФМО, как в фикстурах

    std::promise<std::pair<ScheduleService::GroupLookup, std::optional<int64_t>>>
        p2;
    auto f2 = p2.get_future();
    ScheduleService::groupOfUser(f.db, 5, [&p2](auto lookup, auto groupId) {
        p2.set_value({lookup, groupId});
    });
    const auto staff = f2.get();
    CHECK(staff.first == ScheduleService::GroupLookup::NotFound);
    CHECK_FALSE(staff.second.has_value());
}

TEST_CASE("ScheduleService: today — свой день, свой порядок, чужая группа вне")
{
    Fixture f;
    const auto r = todayOf(f.db, 1, "2026-11-05");
    REQUIRE(r.ok);
    CHECK(r.body["date"] == "2026-11-05");
    const auto& events = r.body["events"];
    REQUIRE(events.size() == 2);  // id 7 (10:00) и id 8 (14:00) — как в
                                  // фикстурах; id 20 группы 2 не включён
    CHECK(idsOf(events) == std::set<int64_t>({7, 8}));
    CHECK(events[0]["starts_at"].asString() == "2026-11-05T10:00:00Z");
    CHECK(events[1]["starts_at"].asString() == "2026-11-05T14:00:00Z");
    // Отменённая пара — прямо в today (todayHasCancelled).
    CHECK(events[0]["status"].asString() == "cancelled");
    CHECK(events[0]["prev_room"].asString() == "2109");
    // Форма события 02 §5.2: дисциплина JOIN'ом. Э6а (гайд §9а):
    // schedule-ответы дополняются teacher_id — «История России» идёт у
    // Симонова (teachers.id 4), фикстурное значение колонки; FE считает
    // people-адрес по Р-15 (teacher_id + kPersonIdOffset).
    CHECK(events[1]["discipline"].asString() == "История России");
    CHECK(events[1]["code"].asString() == "HR-101");
    CHECK(events[1].isMember("teacher_id"));
    CHECK(events[1]["teacher_id"].as<int64_t>() == 4);
    CHECK(events[1]["prev_room"].isNull());

    // Другой день — свои две пары понедельника.
    const auto mon = todayOf(f.db, 1, "2026-11-02");
    REQUIRE(mon.ok);
    CHECK(idsOf(mon.body["events"]) == std::set<int64_t>({1, 2}));

    // Студент другой группы (user 3 → 4-МЭО): только событие своей группы.
    const auto mark = todayOf(f.db, 3, "2026-11-05");
    REQUIRE(mark.ok);
    CHECK(mark.body["date"] == "2026-11-05");
    CHECK(idsOf(mark.body["events"]) == std::set<int64_t>({20}));
}

TEST_CASE("ScheduleService: staff без профиля — 200 с пустым списком")
{
    Fixture f;
    const auto r = todayOf(f.db, 5, "2026-11-05");
    REQUIRE(r.ok);
    CHECK(r.body["date"] == "2026-11-05");
    CHECK(r.body["events"].size() == 0);
    const auto w = weekOf(f.db, 5, 0, nullptr, "2026-11-05");
    REQUIRE(w.ok);
    CHECK(w.body["from"] == "2026-11-02");
    CHECK(w.body["events"].size() == 0);
    const auto c = changesOf(f.db, 5, 7, "2026-11-05");
    REQUIRE(c.ok);
    CHECK(c.body["changes"].size() == 0);
}

TEST_CASE("ScheduleService: week — неделя группы и смещение offset")
{
    Fixture f;
    // offset=0: неделя 02–08.11 (Аня) — все 10 событий группы 1 из фикстур.
    const auto w0 = weekOf(f.db, 1, 0, nullptr, "2026-11-05");
    REQUIRE(w0.ok);
    CHECK(w0.body["offset"] == 0);
    CHECK(w0.body["from"] == "2026-11-02");
    CHECK(w0.body["to"] == "2026-11-08");
    CHECK(w0.body["events"].size() == 10);
    CHECK(idsOf(w0.body["events"]) ==
          std::set<int64_t>({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));
    // Порядок по starts_at внутри недели.
    CHECK(w0.body["events"][0]["id"].as<int64_t>() == 1);
    CHECK(w0.body["events"][9]["id"].as<int64_t>() == 10);
    // Э6а (гайд §9а): week-ответы дополняются teacher_id — фикстурные
    // значения колонки (событие 1 → Симонов teachers.id 4, событие 10 →
    // Рахимова teachers.id 3); FE считает people-адрес по Р-15.
    CHECK(w0.body["events"][0].isMember("teacher_id"));
    CHECK(w0.body["events"][0]["teacher_id"].as<int64_t>() == 4);
    CHECK(w0.body["events"][9]["teacher_id"].as<int64_t>() == 3);

    // offset=1: следующая неделя — пусто.
    const auto w1 = weekOf(f.db, 1, 1, nullptr, "2026-11-05");
    REQUIRE(w1.ok);
    CHECK(w1.body["from"] == "2026-11-09");
    CHECK(w1.body["to"] == "2026-11-15");
    CHECK(w1.body["events"].size() == 0);

    // offset=-1: прошлая неделя — только замещение 01.11 (id 11).
    const auto wm1 = weekOf(f.db, 1, -1, nullptr, "2026-11-05");
    REQUIRE(wm1.ok);
    CHECK(wm1.body["from"] == "2026-10-26");
    CHECK(wm1.body["to"] == "2026-11-01");
    CHECK(idsOf(wm1.body["events"]) == std::set<int64_t>({11}));

    // Неделя другой группы — изоляция (Марк → 4-МЭО, только id 20).
    const auto mark = weekOf(f.db, 3, 0, nullptr, "2026-11-05");
    REQUIRE(mark.ok);
    CHECK(idsOf(mark.body["events"]) == std::set<int64_t>({20}));
}

TEST_CASE("ScheduleService: changes — статусы, окно days, prev_* «было → стало»")
{
    Fixture f;
    const auto r = changesOf(f.db, 1, 7, "2026-11-05");
    REQUIRE(r.ok);
    CHECK(r.body["days"] == 7);
    const auto& changes = r.body["changes"];
    // Окно [30.10, 06.11): два moved 04.11 (id 5,6), cancelled 05.11 (id 7),
    // substituted 01.11 (id 11); normal-события не включаются.
    CHECK(idsOf(changes) == std::set<int64_t>({5, 6, 7, 11}));
    CHECK(changes.size() == 4);
    for (const auto& e : changes)
        CHECK(e["status"].asString() != "normal");

    // «было → стало»: перенос аудитории 3024 → 4210 (С1).
    for (const auto& e : changes)
    {
        if (e["id"].as<int64_t>() != 5)
            continue;
        CHECK(e["status"].asString() == "moved");
        CHECK(e["prev_room"].asString() == "3024");
        CHECK(e["room"].asString() == "4210");
        CHECK(e["prev_starts_at"].asString() == "2026-11-04T10:00:00Z");
        CHECK(e["changed_at"].asString() == "2026-11-02T15:40:00Z");
        CHECK(e["discipline"].asString() == "Мировая экономика");
        CHECK(e["prev_teacher_id"].isNull());
    }
    // Отмена: время не менялось — prev_starts_at == starts_at.
    for (const auto& e : changes)
    {
        if (e["id"].as<int64_t>() != 7)
            continue;
        CHECK(e["status"].asString() == "cancelled");
        CHECK(e["prev_room"].asString() == "2109");
        CHECK(e["prev_starts_at"] == e["starts_at"]);
    }
    // Замещение преподавателя: prev_teacher_id = 6 («было»); Э6а: текущий
    // teacher_id тоже в ответе (замещающий — «стало»).
    for (const auto& e : changes)
    {
        if (e["id"].as<int64_t>() != 11)
            continue;
        CHECK(e["status"].asString() == "substituted");
        CHECK(e["prev_teacher_id"].as<int64_t>() == 6);
        CHECK(e.isMember("teacher_id"));
        CHECK(e["teacher_id"].as<int64_t>() == 1);  // fixture: замещающий id 1
    }

    // Окно days: 1 день → только отменённая пара 05.11; 2 дня → 04–05.11.
    const auto d1 = changesOf(f.db, 1, 1, "2026-11-05");
    REQUIRE(d1.ok);
    CHECK(idsOf(d1.body["changes"]) == std::set<int64_t>({7}));
    const auto d2 = changesOf(f.db, 1, 2, "2026-11-05");
    REQUIRE(d2.ok);
    CHECK(idsOf(d2.body["changes"]) == std::set<int64_t>({5, 6, 7}));
}

TEST_CASE("ScheduleCache: попадание до инвалидации, invalidate(group_id)")
{
    Fixture f;
    // Loop и кэш живут в выделенном потоке: EventLoop обязан создаваться в
    // потоке loop'а (threadId_ фиксируется конструктором), а в потоке теста
    // loop уже есть — статическая инициализация drogon (регистрация
    // контроллеров через app()) создаёт его до main() (проверено
    // getEventLoopOfCurrentThread()): второй EventLoop в этом потоке —
    // FATAL trantor. CacheMap::insert/erase/findAndFetch мьютексом
    // потокобезопасны из любого потока; тикер крутит выделенный loop —
    // это и есть прод-семантика CacheMap (TTL-вытеснение тикает).
    struct CacheEnv
    {
        trantor::EventLoop* loop = nullptr;
        ScheduleCache* cache = nullptr;
    };
    std::promise<CacheEnv> started;
    auto ready = started.get_future();
    std::thread loopThread([&started] {
        trantor::EventLoop loop;      // создаётся В потоке loop'а
        ScheduleCache cache(&loop);   // тикер ставится по runInLoop — в потоке
        started.set_value(CacheEnv{&loop, &cache});
        loop.loop();                  // выходит по quit() из теста
        // loop завершён: runOnQuit CacheMap пометил loopEnded — деструктор
        // не трогает таймер; cache (объявлен позже loop) умрёт раньше loop.
    });
    const auto env = ready.get();
    ScheduleCache* cache = env.cache;  // NOLINT — живёт до join()

    // Промах → выборка → put: после week в кэше модель недели группы 1.
    const auto w1 = weekOf(f.db, 1, 0, cache, "2026-11-05");
    REQUIRE(w1.ok);
    CHECK(w1.body["events"].size() == 10);
    WeekModel cached;
    REQUIRE(cache->get(1, cached));
    CHECK(cached.groupId == 1);
    CHECK(cached.from == "2026-11-02");
    CHECK(cached.to == "2026-11-08");
    CHECK(cached.events.size() == 10);
    CHECK_FALSE(cache->get(2, cached));  // группа 2 не кэшировалась

    // Данные группы изменились под живой записью кэша (добавлено событие):
    // week?offset=0 отдаёт ЗАКЭШИРОВАННУЮ неделю (10, а не 11) — по
    // контракту устаревание снимает инвалидация при переносе (Э6) или TTL.
    insertRawEvent(f.dbPath, 30, "2026-11-03T10:00:00Z", "2026-11-03T11:30:00Z");
    const auto w2 = weekOf(f.db, 1, 0, cache, "2026-11-05");
    REQUIRE(w2.ok);
    CHECK(w2.body["events"].size() == 10);  // из кэша, не пересчёт

    // invalidate(group_id) — хук Э6: следующая week пересчитывает (11).
    cache->invalidate(1);
    CHECK_FALSE(cache->get(1, cached));
    const auto w3 = weekOf(f.db, 1, 0, cache, "2026-11-05");
    REQUIRE(w3.ok);
    CHECK(w3.body["events"].size() == 11);
    REQUIRE(cache->get(1, cached));
    CHECK(cached.events.size() == 11);

    // offset=1 при живом кэше — другая неделя: не берёт кэш группы (0).
    const auto w4 = weekOf(f.db, 1, 1, cache, "2026-11-05");
    REQUIRE(w4.ok);
    CHECK(w4.body["from"] == "2026-11-09");
    CHECK(w4.body["events"].size() == 0);

    cache->invalidate(1);
    CHECK_FALSE(cache->get(1, cached));
    cache->invalidate(1);  // повторная инвалидация отсутствующего — безопасна

    env.loop->quit();  // thread-safe: будит poller, loop() завершается
    loopThread.join();
}

TEST_CASE("ScheduleService: ошибка БД — fail-closed (ok=false → 500)")
{
    // Схема не создана: все выборки падают — сервис отвечает ok=false
    // (контроллер переводит в 500 02 §5.1), а не «пустым расписанием».
    Fixture f(/*withTables=*/false);
    CHECK_FALSE(todayOf(f.db, 1, "2026-11-05").ok);
    CHECK_FALSE(weekOf(f.db, 1, 0, nullptr, "2026-11-05").ok);
    CHECK_FALSE(changesOf(f.db, 1, 7, "2026-11-05").ok);

    std::promise<ScheduleService::GroupLookup> p;
    auto fut = p.get_future();
    ScheduleService::groupOfUser(
        f.db, 1, [&p](auto lookup, auto) { p.set_value(lookup); });
    CHECK(fut.get() == ScheduleService::GroupLookup::Error);
}
