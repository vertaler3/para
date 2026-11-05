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

// Схема — колонки/индексы/CHECK миграции 0003 (02 §4.1); FK не проверяются.
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
             " (application_id, at);");
        sqlite3_close(raw);
    }

    // Прямая вставка заявки в заданном статусе (id → имя для проверок).
    int64_t seedApp(int64_t userId, const std::string& status,
                    const std::string& payload = "{}")
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(), &raw, SQLITE_OPEN_READWRITE,
                                nullptr) == SQLITE_OK);
        exec(raw,
             "INSERT INTO applications (user_id, type, status, payload_json)"
             " VALUES (" + std::to_string(userId) + ", 'study_certificate'"
             ", '" + status + "', '" + payload + "');");
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
