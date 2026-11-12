// Юнит-тесты NewsService — фича 5.17 (GET /v1/news, миграция 0007):
//   - аудитория 'all'|'group'|'course': своя видна, чужая нет, 'all' видят
//     все; профиль сессии отсутствует (staff, Р-6) → видит все;
//   - фильтр ?category= по словарю CHECK 0007 (bogus → BadRequest —
//     400-контракт сервиса, StaffCtrl отдаёт его дословно);
//   - курсор — СОСТАВНОЙ (published_at, id), ровно как у ленты
//     уведомлений: страницы различимы, повтор того же курсора стабилен,
//     хвост — ПУСТАЯ страница (next_cursor null); строка «время против
//     id» не дублируется; чужой по аудитории/несуществующий курсор →
//     BadRequest;
//   - fail-closed: БД без таблиц → cb с Error (future с таймаутом —
//     сервис обязан ответить, а не зависнуть; регресс-паттерн Э8 шага 5).
//
// Схема — DDL миграции 0007 дословно (колонки, словари CHECK, индекс)
// + profiles из 0002 (лента читает группу/курс сессии тем же SELECT,
// что AnnouncementsService::list). Новости ids 1–8 — фикстурные строки
// sql/fixtures.sql (шаг 4, блок 5.17) БАЙТ-В-БАЙТ: расхождений ids нет
// (таблица новая, v2 её не трогала); профили Ани/Марка — реальные
// значения фикстур (Аня 1: группа 1, курс 1; Марк 3: группа 2, курс 4).
// Дополнения теста (свои ids, не из fixtures.sql): 201/202 — пара
// «время против id» (id 201 меньше, но опубликован ПОЗЖЕ 202);
// 101–125 — 25 односекундных audience='all' для постраничности 20+14.
// Временная SQLite через настоящий DbClient — паттерн соседних
// сервисных тестов (notifications_feed_test).
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/NewsService.h"
#include "services/SessionService.h"

namespace fs = std::filesystem;
using para::NewsService;
using para::SessionInfo;

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
               ("para-news-" + std::to_string(stamp) + "-" +
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

struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withTables = true)
    {
        if (withTables)
            createSchema();
        // FK на каждом соединении (Э8 шаг 5) — те же initialization_queries,
        // что в проде (config.json); Drogon 1.9.14 исполняет их на каждом
        // новом соединении пула.
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
        // DDL миграции 0007 (таблица + индекс) — дословно; profiles — колонки
        // миграции 0002 (без FK, как в соседних тестах: лента читает их
        // одним SELECT group_id/course).
        exec(raw,
             "CREATE TABLE news ("
             "  id             INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  category       TEXT NOT NULL CHECK (category IN ('official',"
             "                    'conference','org','event')),"
             "  title          TEXT NOT NULL,"
             "  body           TEXT,"
             "  event_at       TEXT,"
             "  location       TEXT,"
             "  audience       TEXT NOT NULL DEFAULT 'all' CHECK (audience "
             "                    IN ('all','group','course')),"
             "  audience_value TEXT,"
             "  published_at   TEXT NOT NULL"
             ");"
             "CREATE INDEX idx_news_published ON news(published_at DESC, "
             "category);"
             "CREATE TABLE profiles (user_id INTEGER PRIMARY KEY, fio TEXT, "
             "group_id INTEGER, course INTEGER);");
        // Профили — реальные значения фикстур (sql/fixtures.sql: Аня id 1 —
        // группа 1 «1-ФМО», курс 1; Марк id 3 — группа 2 «4-МЭО», курс 4).
        exec(raw,
             "INSERT INTO profiles (user_id, fio, group_id, course) VALUES"
             " (1, 'Соколова Анна Игоревна', 1, 1),"
             " (3, 'Ливанов Марк Сергеевич', 2, 4);");
        // Новости ids 1–8 — блок 5.17 фикстур (шаг 4) БАЙТ-В-БАЙТ: ru/en
        // контент, адресация group '1' (id 5) и course '4' (id 6), будущие
        // события, NULL-поля. Дополнения — ids 201/202 и 101–125.
        exec(raw,
             "INSERT INTO news(id, category, title, body, event_at, "
             "location, audience, audience_value, published_at) VALUES"
             " (1, 'conference', 'Открылась конференция «Международная "
             "неделя МЭО»',"
             "  'Пленарное заседание — 2 ноября в 10:00, Корпус А, ауд. "
             "4210. Впереди пять дней: доклады кафедр, круглые столы и "
             "воркшопы. Секции — по корпусам, расписание — на стенде "
             "деканата ФМО.',"
             "  '2026-11-02T10:00:00Z', 'Корпус А, ауд. 4210', 'all', NULL, "
             "  '2026-11-02T08:30:00Z'),"
             " (2, 'org', 'Киновечер студсовета: «Ретро-осень»',"
             "  'Классика мирового кино из фонда университетской медиатеки: "
             "одна лента — один вечер. 7 ноября, 18:00, Корпус В, ауд. "
             "В-402. Вход по студенческому билету, после сеанса — "
             "обсуждение за чаем.',"
             "  '2026-11-07T18:00:00Z', 'Корпус В, ауд. В-402', 'all', NULL, "
             "  '2026-11-04T12:00:00Z'),"
             " (3, 'event', 'Лекторий «Мировая торговля: новые цепочки "
             "поставок»',"
             "  'Доцент кафедры мировой экономики Павел Егоров — о том, как "
             "перестроилась логистика после 2020 года и что это значит для "
             "региона. 19 ноября, 18:30, Корпус А, ауд. 4212. Регистрация "
             "не нужна, вход свободный.',"
             "  '2026-11-19T18:30:00Z', 'Корпус А, ауд. 4212', 'all', NULL, "
             "  '2026-11-10T09:00:00Z'),"
             " (4, 'event', 'Кубок университета по шахматам',"
             "  'Швейцарская система, 7 туров, контроль 15+10. Регистрация "
             "до 19 ноября у спорторганизатора Корпуса Б. Начало 21 ноября "
             "в 15:00, Корпус Б, ауд. 2224. Победителю — кубок и шахматные "
             "часы.',"
             "  '2026-11-21T15:00:00Z', 'Корпус Б, ауд. 2224', 'all', NULL, "
             "  '2026-11-10T15:00:00Z'),"
             " (5, 'org', 'Собрание группы 1-ФМО: что решили',"
             "  '10 ноября обсудили график консультаций, требования к эссе "
             "и весеннюю практику. Тезисы — у старосты, слайды — в общем "
             "чате группы. Вопросы — куратору потока на неделе.',"
             "  '2026-11-10T17:00:00Z', 'Корпус А, ауд. 3024', 'group', "
             "  '1', '2026-11-10T18:30:00Z'),"
             " (6, 'official', 'Exchange programmes 2027/28: applications "
             "open on 1 December',"
             "  'Semester places at partner universities for the 2027/28 "
             "academic year. Fourth-year students are eligible to apply; a "
             "language certificate is welcome but not required at this "
             "stage. Info session in early December — watch this feed.',"
             "  NULL, NULL, 'course', '4', '2026-11-11T10:00:00Z'),"
             " (7, 'org', 'English Speaking Club: Wednesdays at 18:30',"
             "  'An informal hour of conversation run by the Department of "
             "Foreign Languages — every Wednesday during the term. All "
             "levels are welcome, no sign-up needed.',"
             "  NULL, 'Корпус А, ауд. 2215', 'all', NULL, "
             "  '2026-11-11T16:00:00Z'),"
             " (8, 'official', 'День открытых дверей — 28 ноября',"
             "  'Экскурсии по лабораториям Корпуса В, мини-лекции кафедр и "
             "стенды студенческих объединений. Начало в 11:00, Новый корпус, "
             "ауд. Н-101. Регистрация гостей — у учебного офиса (Корпус А, "
             "ауд. 3101).',"
             "  '2026-11-28T11:00:00Z', 'Новый корпус, ауд. Н-101', 'all', "
             "  NULL, '2026-11-12T09:00:00Z'),"
             // Дополнение 1: «время против id» — 201 опубликован ПОЗЖЕ 202
             // при МЕНЬШЕМ id (проверка составного курсора: порог id<
             // продублировал бы 201 на странице после 202).
             " (201, 'official', 'опубликована 12:00, id меньше',"
             "  'дополнение теста', NULL, NULL, 'all', NULL, "
             "  '2026-11-05T12:00:00Z'),"
             " (202, 'official', 'опубликована 11:59, id больше',"
             "  'дополнение теста', NULL, NULL, 'all', NULL, "
             "  '2026-11-05T11:59:00Z');");
        // Дополнение 2: 25 односекундных audience='all' — постраничность
        // (20+14 у сессии, которой видны все 25).
        std::string bulk =
            "INSERT INTO news(id, category, title, body, event_at, "
            "location, audience, audience_value, published_at) VALUES";
        for (int id = 101; id <= 125; ++id)
        {
            bulk += " (" + std::to_string(id) + ", 'org', 'строка " +
                    std::to_string(id) + "', 'дополнение теста', NULL, "
                    "NULL, 'all', NULL, '2026-11-01T10:00:00Z'),";
        }
        bulk.back() = ';';
        exec(raw, bulk);
        sqlite3_close(raw);
    }
};

// --- Обёртки: cb сервиса → promise/future (паттерн notifications_feed) -----

NewsService::FeedResult feedOf(const drogon::orm::DbClientPtr& db,
                               const SessionInfo& session,
                               const NewsService::Query& q)
{
    std::promise<NewsService::FeedResult> p;
    auto f = p.get_future();
    NewsService::feed(db, session, q,
                      [&p](NewsService::FeedResult r) {
                          p.set_value(std::move(r));
                      });
    // Таймаут — часть контракта fail-closed: cb обязан стрелять, а не
    // оставлять вызывающего висеть (регресс-паттерн Э8 шага 5).
    REQUIRE(f.wait_for(std::chrono::seconds(10)) ==
            std::future_status::ready);
    return f.get();
}

SessionInfo sessionOf(int64_t id, const char* login, const char* role)
{
    SessionInfo s;
    s.userId = id;
    s.login = login;
    s.role = role;
    return s;
}

// Аня (группа 1, курс 1) / Марк (группа 2, курс 4) — профили фикстур;
// dean id 5 — staff БЕЗ профиля (Р-6) в схеме теста.
SessionInfo anya()
{
    return sessionOf(1, "anya.demo", "student");
}
SessionInfo mark()
{
    return sessionOf(3, "mark.demo", "student");
}
SessionInfo dean()
{
    return sessionOf(5, "dean.demo", "staff");
}

// --- Хелперы проверок --------------------------------------------------------

std::vector<int64_t> idsOf(const NewsService::FeedResult& r)
{
    std::vector<int64_t> ids;
    const Json::Value& arr = r.body["news"];
    REQUIRE(arr.isArray());
    for (const auto& item : arr)
        ids.push_back(item["id"].as<int64_t>());
    return ids;
}

// Полный обход ленты по next_cursor (≤ 10 страниц — защита от зацикливания).
std::vector<int64_t> walkAll(const drogon::orm::DbClientPtr& db,
                             const SessionInfo& session)
{
    std::vector<int64_t> all;
    NewsService::Query q;
    for (int page = 0; page < 10; ++page)
    {
        const auto r = feedOf(db, session, q);
        REQUIRE(r.status == NewsService::FeedStatus::Ok);
        const auto ids = idsOf(r);
        all.insert(all.end(), ids.begin(), ids.end());
        if (r.body["next_cursor"].isNull())
            return all;
        q.cursorId = r.body["next_cursor"].as<int64_t>();
    }
    FAIL("лента не закончилась за 10 страниц");
    return all;
}

}  // namespace

TEST_CASE("Лента: аудитория all/group/course — своя видна, чужая нет")
{
    Fixture f;

    // Аня (группа 1, курс 1): видит all (1,2,3,4,7,8) и группу 1 (id 5);
    // курс 4 (id 6) — НЕ видна. Полный обход = 7 фикстурных + 201/202 +
    // 25 односекундных = 34, порядок published_at DESC, id DESC.
    const auto anyaAll = walkAll(f.db, anya());
    CHECK(anyaAll ==
          std::vector<int64_t>({8, 7, 5, 4, 3, 201, 202, 2, 1, 125, 124,
                                123, 122, 121, 120, 119, 118, 117, 116,
                                115, 114, 113, 112, 111, 110, 109, 108,
                                107, 106, 105, 104, 103, 102, 101}));
    CHECK(std::find(anyaAll.begin(), anyaAll.end(), 6) == anyaAll.end());

    // Марк (группа 2, курс 4): видит курс 4 (id 6), группу 1 (id 5) — НЕТ.
    const auto markAll = walkAll(f.db, mark());
    CHECK(markAll.size() == 34);
    CHECK(std::find(markAll.begin(), markAll.end(), 6) != markAll.end());
    CHECK(std::find(markAll.begin(), markAll.end(), 5) == markAll.end());

    // 'all' видят все: фикстурные ids 1–4, 7, 8 — в обеих лентах.
    for (const int64_t id : {1, 2, 3, 4, 7, 8})
    {
        CHECK(std::find(anyaAll.begin(), anyaAll.end(), id) != anyaAll.end());
        CHECK(std::find(markAll.begin(), markAll.end(), id) != markAll.end());
    }

    // staff без профиля (dean id 5, Р-6) — видит ВСЕ, включая 5 и 6.
    const auto deanAll = walkAll(f.db, dean());
    CHECK(deanAll.size() == 35);
    CHECK(std::find(deanAll.begin(), deanAll.end(), 5) != deanAll.end());
    CHECK(std::find(deanAll.begin(), deanAll.end(), 6) != deanAll.end());

    // Поля строки: групповая (5) — audience/audience_value строкой (TEXT
    // 0007) + event_at/location; курсовая (6) — NULL event_at/location →
    // JSON null; все — published_at строкой.
    const auto r5 = feedOf(f.db, anya(), NewsService::Query{});
    REQUIRE(r5.status == NewsService::FeedStatus::Ok);
    const Json::Value& group = r5.body["news"][2];  // порядок выше: 8,7,5
    REQUIRE(group["id"].as<int64_t>() == 5);
    CHECK(group["category"].asString() == "org");
    CHECK(group["audience"].asString() == "group");
    CHECK(group["audience_value"].asString() == "1");
    CHECK(group["event_at"].asString() == "2026-11-10T17:00:00Z");
    CHECK(group["location"].asString() == "Корпус А, ауд. 3024");
    CHECK(group["published_at"].asString() == "2026-11-10T18:30:00Z");

    const auto r6 = feedOf(f.db, mark(), NewsService::Query{});
    REQUIRE(r6.status == NewsService::FeedStatus::Ok);
    const Json::Value& course = r6.body["news"][2];  // порядок: 8,7,6
    REQUIRE(course["id"].as<int64_t>() == 6);
    CHECK(course["audience"].asString() == "course");
    CHECK(course["audience_value"].asString() == "4");
    CHECK(course["event_at"].isNull());
    CHECK(course["location"].isNull());
    CHECK(course["title"].asString() ==
          "Exchange programmes 2027/28: applications open on 1 December");
}

TEST_CASE("Фильтр category: словарь CHECK 0007; вне словаря → BadRequest")
{
    Fixture f;

    // official у Ани: 8 + дополнения 201/202 (курс 4 id 6 скрыта).
    NewsService::Query official;
    official.category = "official";
    auto r = feedOf(f.db, anya(), official);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({8, 201, 202}));

    // official у Марка: + курсовая id 6.
    r = feedOf(f.db, mark(), official);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({8, 6, 201, 202}));

    // event у Ани: только события 4 и 3 (published_at DESC).
    NewsService::Query events;
    events.category = "event";
    r = feedOf(f.db, anya(), events);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({4, 3}));

    // conference у Ани: конференция одна (id 1).
    NewsService::Query conf;
    conf.category = "conference";
    r = feedOf(f.db, anya(), conf);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({1}));

    // Категория вне словаря → BadRequest (400-контракт; стиль ?type у
    // ленты уведомлений) — НЕ пустая выдача и не Error.
    NewsService::Query bogus;
    bogus.category = "bogus";
    r = feedOf(f.db, anya(), bogus);
    CHECK(r.status == NewsService::FeedStatus::BadRequest);
    CHECK_FALSE(r.error.empty());
    CHECK(r.body.isNull());  // частичных данных нет
}

TEST_CASE("Курсор: страницы различимы, стабильность, хвост — пусто")
{
    Fixture f;

    // Аня: полная лента 34 строки → страница 1 = 20 (7 фикстурных + 201/
    // 202 + 2 + 1 + 11 односекундных), next_cursor = id последней строки
    // страницы (115); детектор 21-й строки сработал.
    auto r = feedOf(f.db, anya(), NewsService::Query{});
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    const auto page1 = idsOf(r);
    REQUIRE(page1.size() == 20);
    CHECK(page1.front() == 8);
    CHECK(page1.back() == 115);
    REQUIRE_FALSE(r.body["next_cursor"].isNull());
    CHECK(r.body["next_cursor"].as<int64_t>() == 115);

    // Страница 2: остаток 14 односекундных, следующей страницы нет.
    NewsService::Query next;
    next.cursorId = 115;
    r = feedOf(f.db, anya(), next);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    const auto page2 = idsOf(r);
    CHECK(page2 ==
          std::vector<int64_t>({114, 113, 112, 111, 110, 109, 108, 107,
                                106, 105, 104, 103, 102, 101}));
    CHECK(r.body["next_cursor"].isNull());

    // Страницы различимы: пересечение пусто, объединение = полная лента.
    std::set<int64_t> unionIds(page1.begin(), page1.end());
    unionIds.insert(page2.begin(), page2.end());
    for (const auto id : page2)
        CHECK(std::find(page1.begin(), page1.end(), id) == page1.end());
    CHECK(unionIds.size() == 34);

    // Курсор стабильный: повтор того же запроса — та же страница.
    const auto page2again = idsOf(feedOf(f.db, anya(), next));
    CHECK(page2again == page2);

    // Хвост: курсор последней строки → ПУСТАЯ страница (не Error).
    NewsService::Query tail;
    tail.cursorId = 101;
    r = feedOf(f.db, anya(), tail);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r).empty());
    CHECK(r.body["next_cursor"].isNull());

    // Составной курсор — «время против id»: страница после 202 (11:59)
    // НЕ возвращает 201 (12:00, id меньше) — простой порог id<cursor
    // продублировал бы её. Первая строка хвоста — фикстурная 2 (11-04).
    NewsService::Query tricky;
    tricky.cursorId = 202;
    r = feedOf(f.db, anya(), tricky);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    const auto after = idsOf(r);
    CHECK(after.front() == 2);
    CHECK(std::find(after.begin(), after.end(), 201) == after.end());

    // Несуществующий курсор → BadRequest.
    NewsService::Query missing;
    missing.cursorId = 999;
    CHECK(feedOf(f.db, anya(), missing).status ==
          NewsService::FeedStatus::BadRequest);

    // Чужой по аудитории курсор (курсовая id 6 в сессии Ани) →
    // BadRequest — содержимое невидимых строк не раскрывается.
    NewsService::Query foreign;
    foreign.cursorId = 6;
    CHECK(feedOf(f.db, anya(), foreign).status ==
          NewsService::FeedStatus::BadRequest);
    // Та же строка в ленте Марка — легитимный курсор.
    r = feedOf(f.db, mark(), foreign);
    REQUIRE(r.status == NewsService::FeedStatus::Ok);
    CHECK(idsOf(r).front() == 4);  // после 6 (11-11T10) идут 4 (11-10T15)…
}

TEST_CASE("Fail-closed: БД без таблиц — Error, cb отвечает (не зависает)")
{
    Fixture broken(/*withTables=*/false);

    // Каждый вызов обязан прийти cb с Error (REQUIRE-таймаут обёртки
    // срабатывает только если cb стреляет) — без частичных данных.
    CHECK(feedOf(broken.db, anya(), NewsService::Query{}).status ==
          NewsService::FeedStatus::Error);

    NewsService::Query withCursor;
    withCursor.cursorId = 1;
    CHECK(feedOf(broken.db, anya(), withCursor).status ==
          NewsService::FeedStatus::Error);

    NewsService::Query withCategory;
    withCategory.category = "official";
    CHECK(feedOf(broken.db, dean(), withCategory).status ==
          NewsService::FeedStatus::Error);
}
