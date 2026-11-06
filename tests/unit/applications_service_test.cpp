// Юнит-тесты ApplicationsService (Э5 шаг 2; 02 §5.2, миграция 0003):
//  - POST/создание: без submit (или submit:false) → draft, событий нет;
//    submit:true → submitted + событие подачи NULL→submitted (fixtures.sql,
//    автор — студент);
//  - PUT payload_json: draft → Ok (payload/updated_at обновлены);
//    не-draft → NotDraft (409 по 02 §5.2 «статус ≠ draft»), payload не тронут;
//  - GET {id}: карточка + таймлайн (после submit — событие подачи);
//  - GET список: только свои (изоляция), фильтр по статусу;
//  - messages: текст в таймлайн событием с комментарием, автор — текущий
//    пользователь; на need_info — С7.6: статус возвращается in_progress
//    через автомат шага 1 (матрица need_info→in_progress);
//  - каталог: ключи fixtures.sql (academic_certificate/study_certificate/
//    thesis_topic) входят в kApplicationTypes (03 §4.5.2 — 5 типов);
//  - валидация тел: type вне каталога, payload_json не-JSON, пустой text —
//    parse-* → ok=false (контроллер отдаёт 400);
//  - нет заявки → NotFound; ошибка БД → Error fail-closed.
// Схема — колонки applications/application_events миграции 0003 (как в
// app_workflow_test.cpp); владение в сервисе НЕ проверяется — это работа
// OwnershipGuard в контроллере (правило в шапке OwnershipGuard.h).
#include <chrono>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "services/AppWorkflow.h"

namespace fs = std::filesystem;
using drogon::orm::DrogonDbException;
using para::ApplicationsService;
using para::AppWorkflow;
using para::models::kApplicationStatuses;
using para::models::kApplicationTypes;

namespace
{

struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-appsvc-" + std::to_string(stamp) + "-" +
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

// Схема — колонки/индексы/CHECK миграций 0003 (applications/
// application_events) и 0004 (notifications) + profiles/groups из 0001
// (JOIN очереди сотрудника); FK не проверяются — это работа миграций.
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
             "CREATE TABLE groups ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  name TEXT NOT NULL UNIQUE,"
             "  course INTEGER NOT NULL);"
             "CREATE TABLE profiles ("
             "  user_id INTEGER PRIMARY KEY,"
             "  fio TEXT NOT NULL,"
             "  faculty TEXT NOT NULL,"
             "  program TEXT NOT NULL,"
             "  course INTEGER NOT NULL,"
             "  group_id INTEGER NOT NULL,"
             "  student_card_no TEXT NOT NULL UNIQUE);"
             "INSERT INTO groups(id, name, course) VALUES (1, '1-ФМО', 1),"
             " (2, '4-ФП', 4);"
             "INSERT INTO profiles(user_id, fio, faculty, program, course,"
             " group_id, student_card_no) VALUES"
             " (4, 'Хакимова Дина Ринатовна', 'ФМО', 'Мировая экономика', 1,"
             "  1, '24-1-004'),"
             " (2, 'Соколов Олег Андреевич', 'ФМО', 'Мировая экономика', 1,"
             "  1, '24-1-002');"
             "CREATE TABLE applications ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id INTEGER NOT NULL,"
             "  type TEXT NOT NULL,"
             "  status TEXT NOT NULL DEFAULT 'submitted'"
             "    CHECK (status IN ('draft','submitted','accepted','in_progress',"
             "                      'need_info','ready','issued','rejected','cancelled')),"
             "  payload_json TEXT NOT NULL DEFAULT '{}',"
             "  created_at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')),"
             "  updated_at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE INDEX idx_applications_user ON applications(user_id,"
             " status);"
             "CREATE INDEX idx_applications_queue ON applications(status,"
             " updated_at);"
             "CREATE TABLE application_events ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  application_id INTEGER NOT NULL,"
             "  from_status TEXT,"
             "  to_status TEXT NOT NULL,"
             "  comment TEXT,"
             "  author_id INTEGER,"
             "  at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE INDEX idx_app_events ON application_events"
             " (application_id, at);"
             "CREATE TABLE notifications ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id INTEGER NOT NULL,"
             "  type TEXT NOT NULL CHECK (type IN"
             "    ('schedule','deadline','application','announcement')),"
             "  title TEXT NOT NULL,"
             "  body TEXT,"
             "  deep_link TEXT,"
             "  read_at TEXT,"
             "  created_at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE INDEX idx_notifications_user ON notifications(user_id,"
             " read_at, created_at);");
        sqlite3_close(raw);
    }

    // Прямая вставка заявки в заданном статусе (id → имя для проверок);
    // updatedAt — фиксированное значение для тестов «горит» (пусто → now).
    int64_t seedApp(int64_t userId, const std::string& status,
                    const std::string& payload = "{}",
                    const std::string& type = "study_certificate",
                    const std::string& updatedAt = "")
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READWRITE,
                                nullptr) == SQLITE_OK);
        std::string sql =
            "INSERT INTO applications (user_id, type, status, payload_json";
        if (!updatedAt.empty())
            sql += ", updated_at";
        sql += ") VALUES (" + std::to_string(userId) + ", '" + type + "', '" +
               status + "', '" + payload + "'";
        if (!updatedAt.empty())
            sql += ", '" + updatedAt + "'";
        sql += ");";
        exec(raw, sql);
        const int64_t id = sqlite3_last_insert_rowid(raw);
        sqlite3_close(raw);
        return id;
    }
};

ApplicationsService::Result createOf(
    const drogon::orm::DbClientPtr& db,
    int64_t userId,
    ApplicationsService::CreateSpec spec)
{
    std::promise<ApplicationsService::Result> p;
    auto f = p.get_future();
    ApplicationsService::create(db, userId, spec,
                                [&p](ApplicationsService::Result r) {
                                    p.set_value(std::move(r));
                                });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ApplicationsService::CreateSpec submitSpec(const std::string& type,
                                           bool submit)
{
    ApplicationsService::CreateSpec spec;
    spec.ok = true;
    spec.type = type;
    spec.payloadJson = "{}";
    spec.submit = submit;
    return spec;
}

ApplicationsService::OneResult oneOf(const drogon::orm::DbClientPtr& db,
                                     int64_t appId)
{
    std::promise<ApplicationsService::OneResult> p;
    auto f = p.get_future();
    ApplicationsService::one(db, appId,
                             [&p](ApplicationsService::OneResult r) {
                                 p.set_value(std::move(r));
                             });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ApplicationsService::UpdateResult putOf(const drogon::orm::DbClientPtr& db,
                                        int64_t appId,
                                        const std::string& payloadJson)
{
    std::promise<ApplicationsService::UpdateResult> p;
    auto f = p.get_future();
    ApplicationsService::updateDraft(
        db, appId, payloadJson,
        [&p](ApplicationsService::UpdateResult r) { p.set_value(std::move(r)); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ApplicationsService::Result listOf(
    const drogon::orm::DbClientPtr& db,
    int64_t userId,
    const std::optional<std::string>& statusFilter = std::nullopt)
{
    std::promise<ApplicationsService::Result> p;
    auto f = p.get_future();
    ApplicationsService::list(db, userId, statusFilter,
                              [&p](ApplicationsService::Result r) {
                                  p.set_value(std::move(r));
                              });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ApplicationsService::MessageResult messageOf(const drogon::orm::DbClientPtr& db,
                                             int64_t appId,
                                             const std::string& text,
                                             int64_t author)
{
    std::promise<ApplicationsService::MessageResult> p;
    auto f = p.get_future();
    ApplicationsService::sendMessage(db, appId, text, author,
                                     [&p](ApplicationsService::MessageResult r) {
                                         p.set_value(std::move(r));
                                     });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

const Json::Value* findApp(const Json::Value& arr, int64_t id)
{
    for (const auto& a : arr)
        if (a["id"].as<int64_t>() == id)
            return &a;
    return nullptr;
}

std::string statusTextOf(const drogon::orm::DbClientPtr& db, int64_t appId)
{
    std::promise<std::string> p;
    auto f = p.get_future();
    *db << "SELECT status FROM applications WHERE id = $1" << appId >>
        [&p](const drogon::orm::Result& rows) {
            REQUIRE(rows.size() == 1);
            p.set_value(rows[0]["status"].as<std::string>());
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(
                std::make_exception_ptr(std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

}  // namespace

TEST_CASE("ApplicationsService: каталог — 5 типов 03 §4.5.2, ключи фикстур")
{
    const auto body = ApplicationsService::catalog();
    REQUIRE(body.isMember("types"));
    CHECK(body["types"].size() == 5);
    // Ключи, фигурирующие в sql/fixtures.sql, — в каталоге.
    bool hasAcademic = false, hasStudy = false, hasThesis = false;
    for (const auto& t : body["types"])
    {
        CHECK(t.isMember("key"));
        CHECK(t.isMember("title"));
        const std::string key = t["key"].asString();
        hasAcademic |= key == "academic_certificate";
        hasStudy |= key == "study_certificate";
        hasThesis |= key == "thesis_topic";
    }
    CHECK(hasAcademic);
    CHECK(hasStudy);
    CHECK(hasThesis);
    CHECK(para::models::isKnownApplicationType("academic_certificate"));
    CHECK_FALSE(para::models::isKnownApplicationType("visa_support"));

    // parseCreate: неизвестный ключ → 400, не-JSON payload → 400, submit
    // не-bool → 400; опциональные поля — дефолты.
    auto bad = ApplicationsService::parseCreate(Json::Value());
    CHECK_FALSE(bad.ok);  // нет type
    Json::Value body2;
    body2["type"] = "visa_support";
    bad = ApplicationsService::parseCreate(body2);
    CHECK_FALSE(bad.ok);
    body2["type"] = "academic_certificate";
    body2["payload_json"] = "{\"copies\": не-json";
    bad = ApplicationsService::parseCreate(body2);
    CHECK_FALSE(bad.ok);
    body2["payload_json"] = "{\"copies\":2}";
    body2["submit"] = "yes";
    bad = ApplicationsService::parseCreate(body2);
    CHECK_FALSE(bad.ok);
    body2["submit"] = false;
    const auto good = ApplicationsService::parseCreate(body2);
    CHECK(good.ok);
    CHECK(good.type == "academic_certificate");
    CHECK(good.payloadJson == "{\"copies\":2}");
    CHECK_FALSE(good.submit);
    Json::Value body3;
    body3["type"] = "other";
    const auto minimal = ApplicationsService::parseCreate(body3);
    CHECK(minimal.ok);
    CHECK(minimal.payloadJson == "{}");  // DEFAULT DDL
    CHECK_FALSE(minimal.submit);
}

TEST_CASE("ApplicationsService: POST черновик — draft без событий, PUT обновляет payload")
{
    Fixture f;
    const auto r = createOf(f.db, 4, submitSpec("academic_certificate", false));
    REQUIRE(r.ok);
    CHECK(r.body["status"].asString() == "draft");
    CHECK(r.body["type"].asString() == "academic_certificate");
    CHECK(r.body["type_title"].asString() == "Академическая справка");
    CHECK(r.body["payload"].isObject());
    const int64_t id = r.body["id"].as<int64_t>();

    // Черновик без событий (fixtures.sql: «у Марка-черновика событий нет»).
    const auto before = oneOf(f.db, id);
    REQUIRE(before.status == ApplicationsService::One::Found);
    CHECK(before.body["timeline"].size() == 0);

    // PUT: payload обновлён, статус и updated_at — в ответе.
    const auto put = putOf(f.db, id, "{\"copies\":2,\"delivery\":\"office\"}");
    REQUIRE(put.status == ApplicationsService::Update::Ok);
    CHECK(put.body["status"].asString() == "draft");
    CHECK(put.body["payload"]["copies"].asInt64() == 2);
    CHECK(put.body["payload"]["delivery"].asString() == "office");
    // Повторный PUT (автосохранение раз в 10 с) — тоже Ok.
    CHECK(putOf(f.db, id, "{\"copies\":1}").status ==
          ApplicationsService::Update::Ok);
    // В БД — последнее значение.
    const auto after = oneOf(f.db, id);
    REQUIRE(after.status == ApplicationsService::One::Found);
    CHECK(after.body["payload"]["copies"].asInt64() == 1);
    CHECK(after.body["timeline"].size() == 0);  // PUT событий не пишет
}

TEST_CASE("ApplicationsService: PUT не-draft → NotDraft (409), payload не тронут")
{
    Fixture f;
    // Submitted с payload — PUT должен быть отклонён по статусу.
    const int64_t submitted = f.seedApp(4, "submitted", "{\"copies\":1}");
    // …и черновик для контроля, что 409 именно про статус.
    const int64_t draft = f.seedApp(4, "draft");

    const auto put = putOf(f.db, submitted, "{\"copies\":9}");
    CHECK(put.status == ApplicationsService::Update::NotDraft);
    const auto card = oneOf(f.db, submitted);
    REQUIRE(card.status == ApplicationsService::One::Found);
    CHECK(card.body["payload"]["copies"].asInt64() == 1);  // не перезаписан
    CHECK(card.body["updated_at"].asString() ==
          card.body["created_at"].asString());  // UPDATE не выполнялся

    // Контроль: черновик тем же payload обновляется.
    CHECK(putOf(f.db, draft, "{\"copies\":9}").status ==
          ApplicationsService::Update::Ok);
}

TEST_CASE("ApplicationsService: POST submit:true — submitted + событие подачи")
{
    Fixture f;
    const auto r = createOf(f.db, 4, submitSpec("study_certificate", true));
    REQUIRE(r.ok);
    CHECK(r.body["status"].asString() == "submitted");

    // Таймлайн после submit: ровно одно событие NULL→submitted (fixtures.sql:
    // (1, 1, NULL, 'submitted', NULL, 4, …) — автор студент).
    const auto card = oneOf(f.db, r.body["id"].as<int64_t>());
    REQUIRE(card.status == ApplicationsService::One::Found);
    REQUIRE(card.body["timeline"].size() == 1);
    const auto& ev = card.body["timeline"][0];
    CHECK(ev["application_id"].as<int64_t>() == r.body["id"].as<int64_t>());
    CHECK(ev["from_status"].isNull());
    CHECK(ev["to_status"].asString() == "submitted");
    CHECK(ev["comment"].isNull());
    CHECK(ev["author_id"].as<int64_t>() == 4);
    CHECK(ev["at"].asString().size() == 20);
}

TEST_CASE("ApplicationsService: список — только свои, фильтр по статусу")
{
    Fixture f;
    // Фикстурный расклад: Дина (4) — in_progress; Олег (2) — submitted.
    const int64_t dina = f.seedApp(4, "in_progress");
    const int64_t oleg = f.seedApp(2, "submitted");
    const int64_t dinaDraft = f.seedApp(4, "draft");

    const auto all = listOf(f.db, 4);
    REQUIRE(all.ok);
    CHECK(all.body["applications"].size() == 2);  // свои; Олега нет
    CHECK(findApp(all.body["applications"], oleg) == nullptr);
    const auto* d = findApp(all.body["applications"], dina);
    REQUIRE(d != nullptr);
    CHECK((*d)["status"].asString() == "in_progress");
    CHECK((*d)["type_title"].isString());

    // Фильтр по статусу: только draft Дины.
    const auto drafts = listOf(f.db, 4, std::string("draft"));
    REQUIRE(drafts.ok);
    REQUIRE(drafts.body["applications"].size() == 1);
    CHECK(drafts.body["applications"][0]["id"].as<int64_t>() == dinaDraft);
    CHECK(drafts.body["applications"][0]["status"].asString() == "draft");

    // У пользователя без заявок — пустой список.
    const auto none = listOf(f.db, 9);
    REQUIRE(none.ok);
    CHECK(none.body["applications"].size() == 0);
}

TEST_CASE("ApplicationsService: messages — событие с комментарием, автор — текущий пользователь")
{
    Fixture f;
    const int64_t app = f.seedApp(4, "in_progress");

    const auto r = messageOf(f.db, app, "уточнение по справке", 4);
    REQUIRE(r.status == ApplicationsService::Message::Ok);
    CHECK(r.body["application_id"].as<int64_t>() == app);
    CHECK(r.body["from_status"].asString() == "in_progress");
    CHECK(r.body["to_status"].asString() == "in_progress");  // статус не менялся
    CHECK(r.body["comment"].asString() == "уточнение по справке");
    CHECK(r.body["author_id"].as<int64_t>() == 4);
    CHECK(r.body["at"].asString().size() == 20);

    // Событие — в таймлайне карточки; статус заявки не изменился.
    const auto card = oneOf(f.db, app);
    REQUIRE(card.status == ApplicationsService::One::Found);
    REQUIRE(card.body["timeline"].size() == 1);
    CHECK(card.body["timeline"][0]["comment"].asString() ==
          "уточнение по справке");
    CHECK(statusTextOf(f.db, app) == "in_progress");

    // Пустой/не-строковый text → 400 (parse), не событие.
    Json::Value bad;
    bad["text"] = "   ";
    CHECK_FALSE(ApplicationsService::parseMessageText(bad).ok);
    bad["text"] = 42;
    CHECK_FALSE(ApplicationsService::parseMessageText(bad).ok);
    CHECK_FALSE(ApplicationsService::parsePayloadJson(Json::Value()).ok);
    Json::Value nonJson;
    nonJson["payload_json"] = "не-json";
    CHECK_FALSE(ApplicationsService::parsePayloadJson(nonJson).ok);
}

TEST_CASE("ApplicationsService: С7.6 — ответ на need_info возвращает «В работе»")
{
    Fixture f;
    const int64_t app = f.seedApp(4, "need_info");

    const auto r = messageOf(f.db, app, "кафедра — мировая экономика", 4);
    REQUIRE(r.status == ApplicationsService::Message::Ok);
    // Событие перехода через автомат шага 1: need_info→in_progress,
    // текст ответа — комментарий, автор — студент (С7.6).
    CHECK(r.body["from_status"].asString() == "need_info");
    CHECK(r.body["to_status"].asString() == "in_progress");
    CHECK(r.body["comment"].asString() == "кафедра — мировая экономика");
    CHECK(r.body["author_id"].as<int64_t>() == 4);
    // Статус заявки фактически изменён (матрица шага 1 разрешает переход).
    CHECK(statusTextOf(f.db, app) == "in_progress");

    // Следующее сообщение уже без смены статуса (обычное событие).
    const auto r2 = messageOf(f.db, app, "спасибо", 4);
    REQUIRE(r2.status == ApplicationsService::Message::Ok);
    CHECK(r2.body["from_status"].asString() == "in_progress");
    CHECK(r2.body["to_status"].asString() == "in_progress");
    CHECK(statusTextOf(f.db, app) == "in_progress");
}

TEST_CASE("ApplicationsService: нет заявки → NotFound; ошибка БД → Error")
{
    Fixture f;
    CHECK(oneOf(f.db, 999999).status == ApplicationsService::One::NotFound);
    CHECK(putOf(f.db, 999999, "{}").status ==
          ApplicationsService::Update::NotFound);
    CHECK(messageOf(f.db, 999999, "привет", 4).status ==
          ApplicationsService::Message::NotFound);

    // Схемы нет — fail-closed Error (контроллер → 500), не NotFound.
    Fixture broken(/*withTables=*/false);
    CHECK(oneOf(broken.db, 1).status == ApplicationsService::One::Error);
    CHECK(putOf(broken.db, 1, "{}").status ==
          ApplicationsService::Update::Error);
    CHECK(messageOf(broken.db, 1, "привет", 4).status ==
          ApplicationsService::Message::Error);
    std::promise<ApplicationsService::Result> p;
    auto fut = p.get_future();
    ApplicationsService::create(broken.db, 4, submitSpec("other", false),
                                [&p](ApplicationsService::Result r) {
                                    p.set_value(std::move(r));
                                });
    REQUIRE(fut.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    CHECK_FALSE(fut.get().ok);
    std::promise<ApplicationsService::Result> p2;
    auto fut2 = p2.get_future();
    ApplicationsService::list(broken.db, 4, std::nullopt,
                              [&p2](ApplicationsService::Result r) {
                                  p2.set_value(std::move(r));
                              });
    REQUIRE(fut2.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    CHECK_FALSE(fut2.get().ok);
}

ApplicationsService::Result queueOf(const drogon::orm::DbClientPtr& db,
                                    const ApplicationsService::QueueFilters& filters)
{
    std::promise<ApplicationsService::Result> p;
    auto f = p.get_future();
    ApplicationsService::queue(db, filters, [&p](ApplicationsService::Result r) {
        p.set_value(std::move(r));
    });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

ApplicationsService::StatusChangeResult changeOf(
    const drogon::orm::DbClientPtr& db,
    int64_t appId,
    const std::string& toStatus,
    const std::optional<std::string>& comment,
    int64_t staffUserId)
{
    std::promise<ApplicationsService::StatusChangeResult> p;
    auto f = p.get_future();
    ApplicationsService::staffChangeStatus(
        db, appId, toStatus, comment, staffUserId,
        [&p](ApplicationsService::StatusChangeResult r) {
            p.set_value(std::move(r));
        });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

int64_t countOf(const drogon::orm::DbClientPtr& db, const std::string& table)
{
    std::promise<int64_t> p;
    auto f = p.get_future();
    // table — доверенная константа теста (не пользовательский ввод).
    *db << "SELECT COUNT(*) AS n FROM " + table >>
        [&p](const drogon::orm::Result& rows) {
            p.set_value(rows[0]["n"].as<int64_t>());
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(
                std::make_exception_ptr(std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// Последнее уведомление пользователя (ORDER BY id DESC LIMIT 1) — для
// проверки автоуведомления о смене статуса.
std::unique_ptr<Json::Value> oneNotificationOf(
    const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<std::optional<Json::Value>> p;
    auto f = p.get_future();
    *db << "SELECT id, user_id, type, title, body, deep_link, read_at,"
           " created_at FROM notifications WHERE user_id = $1"
           " ORDER BY id DESC LIMIT 1"
        << userId >>
        [&p](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                p.set_value(std::nullopt);
                return;
            }
            const auto& r = rows[0];
            Json::Value n(Json::objectValue);
            n["id"] = r["id"].as<int64_t>();
            n["user_id"] = r["user_id"].as<int64_t>();
            n["type"] = r["type"].as<std::string>();
            n["title"] = r["title"].as<std::string>();
            if (r["body"].isNull())
                n["body"] = Json::Value();
            else
                n["body"] = r["body"].as<std::string>();
            if (r["deep_link"].isNull())
                n["deep_link"] = Json::Value();
            else
                n["deep_link"] = r["deep_link"].as<std::string>();
            if (r["read_at"].isNull())
                n["read_at"] = Json::Value();
            else
                n["read_at"] = r["read_at"].as<std::string>();
            p.set_value(std::move(n));
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(
                std::make_exception_ptr(std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    auto v = f.get();
    return v ? std::make_unique<Json::Value>(std::move(*v)) : nullptr;
}

// Расклад очереди (Э5 шаг 3): «горит» = 3 дня ожидания в активном статусе —
// старые updated_at ('2026-01-01') против свежих ('2099-01-01', никогда не
// overdue) — детерминированно к любому времени прогона.
TEST_CASE("Staff queue: фильтры status/type/overdue, сортировка, владелец")
{
    Fixture f;
    // ids: 1 — Дина, старая активная (горит); 2 — Олег, свежая; 3 — Дина,
    // старый черновик (draft не горит); 4 — Олег, старая терминальная.
    const int64_t a = f.seedApp(4, "in_progress", "{}", "academic_certificate",
                                "2026-01-01T00:00:00Z");
    const int64_t b = f.seedApp(2, "submitted", "{}", "study_certificate",
                                "2099-01-01T00:00:00Z");
    const int64_t c = f.seedApp(4, "draft", "{}", "thesis_topic",
                                "2026-01-01T00:00:00Z");
    const int64_t d = f.seedApp(2, "issued", "{}", "academic_certificate",
                                "2026-01-01T00:00:00Z");
    CHECK(countOf(f.db, "applications") == 4);

    // Без фильтров: все 4, по updated_at ASC (давние сверху), связи — по id.
    const auto all = queueOf(f.db, {});
    REQUIRE(all.ok);
    REQUIRE(all.body["applications"].size() == 4);
    CHECK(all.body["applications"][0]["id"].as<int64_t>() == a);
    CHECK(all.body["applications"][1]["id"].as<int64_t>() == c);
    CHECK(all.body["applications"][2]["id"].as<int64_t>() == d);
    CHECK(all.body["applications"][3]["id"].as<int64_t>() == b);
    // Владелец (С8.3: профиль в карточке очереди) и признак «горит».
    CHECK(all.body["applications"][0]["owner"]["fio"].asString() ==
          "Хакимова Дина Ринатовна");
    CHECK(all.body["applications"][0]["owner"]["group"].asString() == "1-ФМО");
    CHECK(all.body["applications"][1]["owner"]["fio"].asString() ==
          "Хакимова Дина Ринатовна");
    CHECK(all.body["applications"][3]["owner"]["fio"].asString() ==
          "Соколов Олег Андреевич");
    CHECK(all.body["applications"][0]["overdue"].asBool() == true);
    CHECK(all.body["applications"][1]["overdue"].asBool() == false);  // draft
    CHECK(all.body["applications"][2]["overdue"].asBool() == false);  // issued
    CHECK(all.body["applications"][3]["overdue"].asBool() == false);  // свежая

    // Фильтр статуса.
    ApplicationsService::QueueFilters byStatusF;
    byStatusF.status = "in_progress";
    const auto byStatus = queueOf(f.db, byStatusF);
    REQUIRE(byStatus.ok);
    REQUIRE(byStatus.body["applications"].size() == 1);
    CHECK(byStatus.body["applications"][0]["id"].as<int64_t>() == a);

    // Фильтр типа.
    ApplicationsService::QueueFilters byTypeF;
    byTypeF.type = "academic_certificate";
    const auto byType = queueOf(f.db, byTypeF);
    REQUIRE(byType.ok);
    REQUIRE(byType.body["applications"].size() == 2);
    CHECK(byType.body["applications"][0]["id"].as<int64_t>() == a);
    CHECK(byType.body["applications"][1]["id"].as<int64_t>() == d);

    // overdue=1: только старая активная; draft/issued/свежая — мимо.
    ApplicationsService::QueueFilters overdueF;
    overdueF.overdueOnly = true;
    const auto overdue = queueOf(f.db, overdueF);
    REQUIRE(overdue.ok);
    REQUIRE(overdue.body["applications"].size() == 1);
    CHECK(overdue.body["applications"][0]["id"].as<int64_t>() == a);
    CHECK(overdue.body["applications"][0]["overdue"].asBool() == true);

    // Комбинация overdue + status.
    ApplicationsService::QueueFilters comboF;
    comboF.status = "submitted";
    comboF.overdueOnly = true;
    const auto combo = queueOf(f.db, comboF);
    REQUIRE(combo.ok);
    CHECK(combo.body["applications"].size() == 0);  // submitted — только свежая
}

TEST_CASE("Staff status change: автомат + событие + автоуведомление автору")
{
    Fixture f;
    const int64_t app = f.seedApp(4, "in_progress", "{}",
                                  "academic_certificate");
    REQUIRE(countOf(f.db, "notifications") == 0);
    REQUIRE(countOf(f.db, "application_events") == 0);

    // in_progress → ready (валидный переход): статус, событие, уведомление.
    const auto r = changeOf(f.db, app, "ready", std::nullopt, 5);
    REQUIRE(r.status == ApplicationsService::StatusChange::Ok);
    CHECK(r.body["status"].asString() == "ready");
    CHECK(statusTextOf(f.db, app) == "ready");
    CHECK(countOf(f.db, "application_events") == 1);
    // Уведомление автору заявки (Дине, user 4), не сотруднику (02 §4.1):
    // type 'application', title «Тип: Статус» по меткам models, deep_link
    // на карточку заявки (4.5), непрочитанное.
    REQUIRE(countOf(f.db, "notifications") == 1);
    const auto n = oneNotificationOf(f.db, 4);
    REQUIRE(n != nullptr);
    CHECK((*n)["user_id"].as<int64_t>() == 4);
    CHECK((*n)["type"].asString() == "application");
    CHECK((*n)["title"].asString() == "Академическая справка: Готова");
    CHECK((*n)["body"].isNull());  // комментарий не передан
    CHECK((*n)["deep_link"].asString() == "/applications/" +
                                              std::to_string(app));
    CHECK((*n)["read_at"].isNull());

    // rejected с причиной (из in_progress — из ready только issued, 05 §3
    // Э5): комментарий — в body уведомления («rejected (с причиной)»).
    const int64_t app2 = f.seedApp(2, "in_progress", "{}",
                                   "study_certificate");
    const auto r2 = changeOf(f.db, app2, "rejected",
                             std::string("не выдаётся в демо-контуре"), 5);
    REQUIRE(r2.status == ApplicationsService::StatusChange::Ok);
    CHECK(statusTextOf(f.db, app2) == "rejected");
    REQUIRE(countOf(f.db, "notifications") == 2);
    const auto n2 = oneNotificationOf(f.db, 2);
    REQUIRE(n2 != nullptr);
    CHECK((*n2)["user_id"].as<int64_t>() == 2);  // автору этой заявки
    CHECK((*n2)["title"].asString() == "Справка об обучении: Отклонена");
    CHECK((*n2)["body"].asString() == "не выдаётся в демо-контуре");
    CHECK(countOf(f.db, "application_events") == 2);

    // Недопустимый переход (rejected → in_progress вне матрицы): 409 и БЕЗ
    // уведомления/события — транзакция откатывается целиком.
    const auto bad = changeOf(f.db, app2, "in_progress", std::nullopt, 5);
    CHECK(bad.status == ApplicationsService::StatusChange::InvalidTransition);
    CHECK(statusTextOf(f.db, app2) == "rejected");
    CHECK(countOf(f.db, "notifications") == 2);
    CHECK(countOf(f.db, "application_events") == 2);

    // Несуществующая заявка → NotFound (OwnershipGuard не нужен — staff
    // видит все; 404 различается от 409).
    CHECK(changeOf(f.db, 999999, "accepted", std::nullopt, 5).status ==
          ApplicationsService::StatusChange::NotFound);
    CHECK(countOf(f.db, "notifications") == 2);

    // Ошибка БД — fail-closed: Error, ничего не записано.
    Fixture broken(/*withTables=*/false);
    CHECK(changeOf(broken.db, app, "ready", std::nullopt, 5).status ==
          ApplicationsService::StatusChange::Error);
}
