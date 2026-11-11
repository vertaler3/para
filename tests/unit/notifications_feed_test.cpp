// Юнит-тесты NotificationsFeedService — закрытие пробела Э6 (02 §5.2):
//   GET  /v1/notifications           — лента текущего пользователя
//       (ORDER BY created_at DESC, id DESC; query unread=1/type/cursor);
//   POST /v1/notifications/{id}/read — через OwnershipGuard
//       (своё → Own → 204-цепочка, чужое → Forbidden/403 ДО пометки,
//       несуществующее → NotFound/404, ошибка БД → fail-closed);
//       идемпотентно: повторный read — AlreadyRead, метка read_at не
//       дублируется/не перезаписывается;
//   POST /v1/notifications/read-all  — помечает всё СВОЁ (marked = число
//       непрочитанных), чужие не трогаются, повторный — marked=0.
//
// Проверки:
//  - лента-изоляция: у Ани и Олега ЗАГОЛОВКИ фикстур совпадают дословно
//    (как в sql/fixtures.sql: рассылка переноса каждому студенту группы) —
//    изоляция доказывается составом id, не заголовками;
//  - unread-фильтр: после read строка уходит из unread, остаётся в полной;
//  - read-idempotent: второй markRead — AlreadyRead, read_at не изменился;
//  - read-all помечает всё (и только своё);
//  - type-фильтр — словарь CHECK 0004 (bogus → BadRequest);
//  - курсор — СОСТАВНОЙ (created_at, id): строка «время против id»
//    (id 101 создан 12:00, id 102 — 11:59) не дублируется на странице 2
//    (простой порог id<cursor вернул бы её повторно); чужой/несуществующий
//    курсор → BadRequest; постраничность 20+5 на 25 односекундных строках;
//  - fail-closed: без таблиц → Error/ok=false.
// Схема — колонки notifications из миграции 0004 (CHECK и индекс — как в
// DDL; FK не нужны: guard читает user_id самой строки). Временная SQLite
// через настоящий DbClient — как в тестах Э2–Э6.
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <future>
#include <set>
#include <string>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "services/NotificationsFeedService.h"
#include "services/OwnershipGuard.h"

namespace fs = std::filesystem;
using para::NotificationsFeedService;
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
               ("para-notifyfeed-" + std::to_string(stamp) + "-" +
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
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }

    void createSchema()
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        // DDL миграции 0004 (колонки, CHECK-словарь, индекс) — дословно.
        exec(raw,
             "CREATE TABLE notifications ("
             "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id    INTEGER NOT NULL,"
             "  type       TEXT NOT NULL CHECK (type IN ('schedule','deadline',"
             "                    'application','announcement')),"
             "  title      TEXT NOT NULL,"
             "  body       TEXT,"
             "  deep_link  TEXT,"
             "  read_at    TEXT,"
             "  created_at TEXT NOT NULL DEFAULT "
             "             (strftime('%Y-%m-%dT%H:%M:%SZ','now'))"
             ");"
             "CREATE INDEX idx_notifications_user "
             "ON notifications(user_id, read_at, created_at);");
        // Демо-уведомления sql/fixtures.sql дословно (ids 1–6: рассылка
        // переноса каждому студенту группы — заголовки Ани и Олега
        // СОВПАДАЮТ) + помеченные дополнения: дедлайн и прочитанное
        // application у Ани; «время против id» и NULL-поля у user 9;
        // 25 односекундных строк user 10 — постраничность.
        // Фикстуры v2 (10.11.2026) ids 1–6 не тронули и добавили свои
        // дедлайн-уведомления ids 7–8 (Аня/Марк, deep_link '/deadlines') —
        // они в тестовую БД НЕ копируются; номера 7/8 заняты дополнениями
        // теста только в её собственной временной БД (в fixtures.sql это
        // другие строки: другой title/created_at/deep_link).
        exec(raw,
             "INSERT INTO notifications(id, user_id, type, title, body, "
             "deep_link, read_at, created_at) VALUES"
             " (1, 1, 'schedule', 'Пара в 10:00 перенесена в ауд. 4210',"
             "  'Мировая экономика, 4 ноября: ауд. 3024 → 4210',"
             "  '/schedule?date=2026-11-04', NULL, '2026-11-02T15:40:00Z'),"
             " (2, 1, 'schedule', 'Пара в 14:00 перенесена в ауд. 4210',"
             "  'Философия, 4 ноября: ауд. 3105 → 4210',"
             "  '/schedule?date=2026-11-04', NULL, '2026-11-02T15:41:00Z'),"
             " (3, 1, 'schedule', 'Пара отменена: История МЭО, чт 10:00',"
             "  'Лекция 5 ноября отменена деканатом',"
             "  '/schedule?date=2026-11-05', NULL, '2026-11-02T15:45:00Z'),"
             " (4, 2, 'schedule', 'Пара в 10:00 перенесена в ауд. 4210',"
             "  'Мировая экономика, 4 ноября: ауд. 3024 → 4210',"
             "  '/schedule?date=2026-11-04', NULL, '2026-11-02T15:40:00Z'),"
             " (5, 2, 'schedule', 'Пара в 14:00 перенесена в ауд. 4210',"
             "  'Философия, 4 ноября: ауд. 3105 → 4210',"
             "  '/schedule?date=2026-11-04', NULL, '2026-11-02T15:41:00Z'),"
             " (6, 2, 'schedule', 'Пара отменена: История МЭО, чт 10:00',"
             "  'Лекция 5 ноября отменена деканатом',"
             "  '/schedule?date=2026-11-05', NULL, '2026-11-02T15:45:00Z'),"
             " (7, 1, 'deadline', 'Дедлайн: заявка на справку',"
             "  'Завтра 18:00 — деканат (концепт, вымышленные данные)',"
             "  '/applications/1', NULL, '2026-11-05T09:00:00Z'),"
             " (8, 1, 'application', 'Справка об обучении: Принята',"
             "  'Заявка №1 принята в работу',"
             "  '/applications/1', '2026-11-06T10:00:00Z',"
             "  '2026-11-06T09:30:00Z'),"
             " (101, 9, 'application', 'создана 12:00, id меньше',"
             "  NULL, NULL, NULL, '2026-11-07T12:00:00Z'),"
             " (102, 9, 'application', 'создана 11:59, id больше',"
             "  NULL, NULL, NULL, '2026-11-07T11:59:00Z'),"
             " (103, 9, 'deadline', 'Без тела',"
             "  NULL, NULL, NULL, '2026-11-07T11:00:00Z')");
        std::string bulk =
            "INSERT INTO notifications(id, user_id, type, title, body, "
            "deep_link, read_at, created_at) VALUES";
        for (int id = 201; id <= 225; ++id)
        {
            bulk += " (" + std::to_string(id) + ", 10, 'announcement',"
                    " 'строка " + std::to_string(id) + "',"
                    " 'объявление', NULL, NULL, '2026-11-07T10:00:00Z'),";
        }
        bulk.back() = ';';
        exec(raw, bulk);
        sqlite3_close(raw);
    }
};

// --- Обёртки: cb сервиса → promise/future (паттерн grades_service_test) -----

NotificationsFeedService::FeedResult
feedOf(const drogon::orm::DbClientPtr& db,
       int64_t userId,
       const NotificationsFeedService::Query& q)
{
    std::promise<NotificationsFeedService::FeedResult> p;
    auto f = p.get_future();
    NotificationsFeedService::feed(db, userId, q,
                                   [&p](NotificationsFeedService::FeedResult r) {
                                       p.set_value(std::move(r));
                                   });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

NotificationsFeedService::ReadStatus
readOf(const drogon::orm::DbClientPtr& db, int64_t userId, int64_t id)
{
    std::promise<NotificationsFeedService::ReadStatus> p;
    auto f = p.get_future();
    NotificationsFeedService::markRead(db, userId, id,
                                       [&p](NotificationsFeedService::ReadStatus s) {
                                           p.set_value(s);
                                       });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

NotificationsFeedService::MarkAllResult
readAllOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<NotificationsFeedService::MarkAllResult> p;
    auto f = p.get_future();
    NotificationsFeedService::markAllRead(db, userId,
                                          [&p](NotificationsFeedService::MarkAllResult r) {
                                              p.set_value(r);
                                          });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

OwnershipGuard::Verdict guardOf(const drogon::orm::DbClientPtr& db,
                                int64_t userId,
                                int64_t notificationId)
{
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db, OwnershipGuard::Table::Notifications, userId,
                          notificationId,
                          [&p](OwnershipGuard::Verdict v) { p.set_value(v); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// --- Хелперы проверок --------------------------------------------------------

std::vector<int64_t> idsOf(const NotificationsFeedService::FeedResult& r)
{
    std::vector<int64_t> ids;
    const Json::Value& arr = r.body["notifications"];
    REQUIRE(arr.isArray());
    for (const auto& item : arr)
        ids.push_back(item["id"].as<int64_t>());
    return ids;
}

std::string readAtOf(const drogon::orm::DbClientPtr& db, int64_t id)
{
    std::promise<std::string> p;
    auto f = p.get_future();
    // Сырой SELECT метки — источник истины (не через сервис).
    *db << "SELECT read_at FROM notifications WHERE id = $1" << id >>
        [&p](const drogon::orm::Result& rows) {
            p.set_value(rows.empty() ? std::string("<нет строки>")
                                     : (rows[0]["read_at"].isNull()
                                            ? std::string("<NULL>")
                                            : rows[0]["read_at"]
                                                  .as<std::string>()));
        } >>
        [&p](const drogon::orm::DrogonDbException& e) {
            FAIL(e.base().what());
            p.set_value("<error>");
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

}  // namespace

TEST_CASE("Лента: свои строки, новые сверху, изоляция, поля строки")
{
    Fixture f;

    // Аня (user 1): 5 строк в порядке created_at DESC (created_at DESC —
    // id DESC тирбейк: 3 (15:45) → 2 (15:41) → 1 (15:40)).
    auto anya = feedOf(f.db, 1, {});
    REQUIRE(anya.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(anya) == std::vector<int64_t>({8, 7, 3, 2, 1}));
    CHECK(anya.body["next_cursor"].isNull());

    // Поля строки: прочитанная (8) — read_at из DDL-формата; непрочитанная
    // (1) — read_at null, title/body/deep_link — строки фикстур.
    const Json::Value& read = anya.body["notifications"][0];
    CHECK(read["id"].as<int64_t>() == 8);
    CHECK(read["type"].asString() == "application");
    CHECK(read["read_at"].asString() == "2026-11-06T10:00:00Z");
    CHECK(read["created_at"].asString() == "2026-11-06T09:30:00Z");
    const Json::Value& fresh = anya.body["notifications"][3];
    CHECK(fresh["id"].as<int64_t>() == 2);
    CHECK(fresh["type"].asString() == "schedule");
    CHECK(fresh["read_at"].isNull());
    CHECK(fresh["title"].asString() == "Пара в 14:00 перенесена в ауд. 4210");
    CHECK(fresh["deep_link"].asString() == "/schedule?date=2026-11-04");

    // Изоляция: у Олега (user 2) — только его 3 строки (заголовки те же,
    // что у Ани 1–3, — совпадение доказывает именно состав id).
    auto oleg = feedOf(f.db, 2, {});
    REQUIRE(oleg.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(oleg) == std::vector<int64_t>({6, 5, 4}));

    // Пользователь без уведомлений — пустой список (не Error).
    auto empty = feedOf(f.db, 99, {});
    REQUIRE(empty.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(empty).empty());
    CHECK(empty.body["next_cursor"].isNull());

    // NULL-поля → JSON null (строка 103: body/deep_link/read_at NULL).
    auto u9 = feedOf(f.db, 9, {});
    REQUIRE(u9.status == NotificationsFeedService::FeedStatus::Ok);
    // created_at DESC: 101 (12:00) → 102 (11:59) → 103 (11:00) — порядок
    // времени против порядка id (101 < 102, но новее).
    CHECK(idsOf(u9) == std::vector<int64_t>({101, 102, 103}));
    const Json::Value& noBody = u9.body["notifications"][2];
    CHECK(noBody["id"].as<int64_t>() == 103);
    CHECK(noBody["body"].isNull());
    CHECK(noBody["deep_link"].isNull());
    CHECK(noBody["read_at"].isNull());
}

TEST_CASE("Фильтры unread=1 и type; type вне словаря → BadRequest")
{
    Fixture f;

    // unread=1: прочитанная (8) исключена, остальные по времени.
    NotificationsFeedService::Query unread;
    unread.unreadOnly = true;
    auto r = feedOf(f.db, 1, unread);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({7, 3, 2, 1}));

    // type-фильтр по словарю CHECK 0004.
    NotificationsFeedService::Query sched;
    sched.type = "schedule";
    r = feedOf(f.db, 1, sched);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({3, 2, 1}));

    NotificationsFeedService::Query dl;
    dl.type = "deadline";
    r = feedOf(f.db, 1, dl);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({7}));

    NotificationsFeedService::Query ann;
    ann.type = "announcement";
    r = feedOf(f.db, 1, ann);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r).empty());  // своих announcements нет — пусто, не Error

    // Комбинация unread+type.
    NotificationsFeedService::Query unreadSched;
    unreadSched.unreadOnly = true;
    unreadSched.type = "schedule";
    r = feedOf(f.db, 1, unreadSched);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({3, 2, 1}));

    // type вне словаря → BadRequest (02 §5.1, стиль ?status=bogus → 400).
    NotificationsFeedService::Query bogus;
    bogus.type = "bogus";
    r = feedOf(f.db, 1, bogus);
    CHECK(r.status == NotificationsFeedService::FeedStatus::BadRequest);
    CHECK_FALSE(r.error.empty());

    // user 10 — 25 announcements: type-фильтр отдаёт все 25 (одна страница
    // ограничена kPageSize — 20, постраничность — кейс курсора ниже).
    NotificationsFeedService::Query annAll;
    annAll.type = "announcement";
    r = feedOf(f.db, 10, annAll);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r).size() == 20);
}

TEST_CASE("read: идемпотентность; guard Table::Notifications 403/404")
{
    Fixture f;

    // Цепочка контроллера (guard → markRead) для СВОЕЙ строки.
    REQUIRE(guardOf(f.db, 1, 2) == OwnershipGuard::Verdict::Own);
    CHECK(readOf(f.db, 1, 2) == NotificationsFeedService::ReadStatus::Ok);
    const std::string firstStamp = readAtOf(f.db, 2);
    CHECK(firstStamp != "<NULL>");

    // Повторный read — уже прочитанное: без ошибки (AlreadyRead → тот же
    // 204 контроллером), метка НЕ перезаписывается (условный UPDATE
    // read_at IS NULL; серверное время 1 с — равенство строк доказывает
    // отсутствие повторной записи).
    REQUIRE(guardOf(f.db, 1, 2) == OwnershipGuard::Verdict::Own);
    CHECK(readOf(f.db, 1, 2) ==
          NotificationsFeedService::ReadStatus::AlreadyRead);
    CHECK(readAtOf(f.db, 2) == firstStamp);

    // Строка ушла из unread, осталась в полной ленте.
    NotificationsFeedService::Query unread;
    unread.unreadOnly = true;
    auto r = feedOf(f.db, 1, unread);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    const auto ids = idsOf(r);
    CHECK(std::find(ids.begin(), ids.end(), 2) == ids.end());
    NotificationsFeedService::Query none;
    r = feedOf(f.db, 1, none);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r).size() == 5);

    // Guard: чужая существующая (2 → чужая id 4 Олега) → Forbidden (403 ДО
    // пометки); несуществующая → NotFound (404) для любого пользователя.
    CHECK(guardOf(f.db, 2, 4) == OwnershipGuard::Verdict::Own);
    CHECK(guardOf(f.db, 1, 4) == OwnershipGuard::Verdict::Forbidden);
    CHECK(guardOf(f.db, 2, 1) == OwnershipGuard::Verdict::Forbidden);
    CHECK(guardOf(f.db, 1, 999) == OwnershipGuard::Verdict::NotFound);
    CHECK(guardOf(f.db, 2, 999) == OwnershipGuard::Verdict::NotFound);
    // После 403-вердикта контроллер markRead не вызывает — прямым вызовом
    // сервиса чужое не помечается (скоуп user_id в WHERE, 02 §6.3 п.2):
    CHECK(readOf(f.db, 1, 4) ==
          NotificationsFeedService::ReadStatus::AlreadyRead);
    CHECK(readAtOf(f.db, 4) == "<NULL>");
}

TEST_CASE("read-all: помечает всё СВОЁ, повторный — marked=0, чужие нет")
{
    Fixture f;

    auto r = readAllOf(f.db, 1);
    CHECK(r.ok);
    CHECK(r.marked == 4);  // у Ани 4 непрочитанных (8 уже прочитано)

    NotificationsFeedService::Query unread;
    unread.unreadOnly = true;
    auto feed = feedOf(f.db, 1, unread);
    REQUIRE(feed.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(feed).empty());

    // Повторный read-all — идемпотентен: 0 строк (тот же 204).
    r = readAllOf(f.db, 1);
    CHECK(r.ok);
    CHECK(r.marked == 0);

    // Полная лента на месте, все строки получили read_at.
    NotificationsFeedService::Query none;
    feed = feedOf(f.db, 1, none);
    REQUIRE(feed.status == NotificationsFeedService::FeedStatus::Ok);
    const auto all = idsOf(feed);
    CHECK(all.size() == 5);
    for (const auto id : all)
        CHECK(readAtOf(f.db, id) != "<NULL>");

    // read-all Ани не тронул Олега: его 3 строки остались непрочитанными.
    feed = feedOf(f.db, 2, unread);
    REQUIRE(feed.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(feed) == std::vector<int64_t>({6, 5, 4}));
}

TEST_CASE("Курсор: составная пагинация (created_at, id), 20+5, BadRequest")
{
    Fixture f;

    // Составной курсор — «время против id»: страница с курсором 102
    // (created 11:59) НЕ возвращает 101 (создан 12:00, id 101 < 102 —
    // простой порог id<cursor продублировал бы его: 101 уже был первой
    // строкой страницы). created_at < '11:59' → только 103.
    NotificationsFeedService::Query cur;
    cur.cursorId = 102;
    auto r = feedOf(f.db, 9, cur);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    CHECK(idsOf(r) == std::vector<int64_t>({103}));
    CHECK(r.body["next_cursor"].isNull());

    // Постраничность: 25 односекундных строк user 10 → страница 1 — 20
    // строк (ids 225…206 по id DESC при равном created_at), next_cursor =
    // id последней строки страницы (206); страница 2 — остаток 5.
    NotificationsFeedService::Query none;
    r = feedOf(f.db, 10, none);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    const auto page1 = idsOf(r);
    REQUIRE(page1.size() == 20);
    CHECK(page1.front() == 225);
    CHECK(page1.back() == 206);
    REQUIRE_FALSE(r.body["next_cursor"].isNull());
    CHECK(r.body["next_cursor"].as<int64_t>() == 206);

    NotificationsFeedService::Query next;
    next.cursorId = r.body["next_cursor"].as<int64_t>();
    r = feedOf(f.db, 10, next);
    REQUIRE(r.status == NotificationsFeedService::FeedStatus::Ok);
    const auto page2 = idsOf(r);
    CHECK(page2 == std::vector<int64_t>({205, 204, 203, 202, 201}));
    CHECK(r.body["next_cursor"].isNull());

    // Несуществующий курсор → BadRequest.
    NotificationsFeedService::Query missing;
    missing.cursorId = 999;
    r = feedOf(f.db, 1, missing);
    CHECK(r.status == NotificationsFeedService::FeedStatus::BadRequest);

    // Чужой курсор (id 4 Олега в сессии Ани) → BadRequest — содержимое
    // чужих строк не раскрывается.
    NotificationsFeedService::Query foreign;
    foreign.cursorId = 4;
    r = feedOf(f.db, 1, foreign);
    CHECK(r.status == NotificationsFeedService::FeedStatus::BadRequest);
}

TEST_CASE("Fail-closed: ошибка БД — Error/ok=false, не пустой успех")
{
    Fixture broken(/*withTables=*/false);

    auto r = feedOf(broken.db, 1, {});
    CHECK(r.status == NotificationsFeedService::FeedStatus::Error);

    NotificationsFeedService::Query unread;
    unread.unreadOnly = true;
    CHECK(feedOf(broken.db, 1, unread).status ==
          NotificationsFeedService::FeedStatus::Error);

    CHECK(readOf(broken.db, 1, 1) ==
          NotificationsFeedService::ReadStatus::Error);

    auto all = readAllOf(broken.db, 1);
    CHECK_FALSE(all.ok);
    CHECK(all.marked == 0);
}
