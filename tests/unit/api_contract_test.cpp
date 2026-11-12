// Юнит-тесты КОНТРОЛЛЕРНОГО контракта фич э7-be шагов 1–2 (шаг 6):
//  - PUT /v1/applications/{id} submit:true (8.i, Р-29): своя draft → 200
//    (status submitted, payload обновлён, событие draft→submitted с датой);
//    не-draft → 409 {"error":{"code":"invalid_status_transition"}} (02
//    §5.2/§5.1); чужая → 403 forbidden — guard ДО разбора тела и ДО
//    проверки статуса (ApplicationsCtrl.cpp: guard→тело→статус), поэтому
//    чужая DRAFT — это 403, а не 200/409, и чужая без JSON-тела — 403,
//    а не 400; несуществующая → 404 not_found.
//  - GET /v1/ics/token (8.ii): активный токен → 200 {token, created_at}
//    (ровно два поля); нет строки ИЛИ отозван → 404 not_found.
//
// Паттерн StaffFilter-теста (rooms_service_test.cpp): метод контроллера
// вызывается НАПРЯМУЮ с HttpRequest — атрибут "session" в проде кладёт
// LoginFilter; ApplicationsCtrl/IcsCtrl линкуются в para-tests (шаг 6).
// Сервисный уровень (enum Update::NotDraft, ActiveToken, даты событий,
// parseUpdate) уже покрыт шагами 1–2 в applications_service_test /
// ics_service_token_test — здесь только HTTP-маппинг слоем выше, без
// дублирования.
//
// Контроллеры берут app().getDbClient("para"), а Drogon 1.9.14 создаёт
// клиентов только при старте приложения (addDbClient хранит конфиг;
// DbClientManager.h: dbClientsMap_ заполняет createDbClients(ioLoops) из
// run()) — поэтому Env поднимает app() в отдельном потоке по конфигу на
// временной БД (рецепт security_tests: findFreePort + writeTestConfig +
// ServerGuard). HTTP-запросы НЕ отправляются: слушатель нужен только,
// чтобы run() стартовал; проверяемый слой — методы контроллера.
// Схема/фикстуры вносятся ТЕМ ЖЕ db-клиентом (одно соединение пула —
// без межсоединительных SQLITE_BUSY и без raw-sqlite до старта Drogon,
// который ломал бы sqlite3_config).
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <doctest.hpp>

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include "controllers/ApplicationsCtrl.h"
#include "controllers/IcsCtrl.h"
#include "services/ApplicationsService.h"
#include "services/SessionService.h"

namespace fs = std::filesystem;
using para::ApplicationsService;
using para::SessionInfo;

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
               ("para-apicontract-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

// Свободный порт для слушателя: bind :0 → getsockname → отпустить
// (security_tests.cpp:391 — тот же приём).
unsigned short findFreePort()
{
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return 0;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    bool ok = ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    if (ok)
        ok = ::listen(fd, 1) == 0;
    socklen_t len = sizeof(addr);
    if (ok)
        ok = ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == 0;
    ::close(fd);
    return ok ? ntohs(addr.sin_port) : 0;
}

// app().run() в отдельном потоке; деструктор останавливает (ServerGuard
// из security_tests.cpp:664).
struct ServerGuard
{
    std::thread thread;
    ServerGuard() : thread([] { drogon::app().run(); }) {}
    ~ServerGuard()
    {
        drogon::app().quit();
        if (thread.joinable())
            thread.join();
    }
    ServerGuard(const ServerGuard&) = delete;
    ServerGuard& operator=(const ServerGuard&) = delete;
};

// Схема — applications/application_events миграции 0003 (как в
// applications_service_test.cpp) + ics_tokens миграции 0004 (как в
// ics_service_token_test.cpp); профили/users не нужны — UPDATE/SELECT
// пути этих контрактов их не трогают.
//
// Одна БД на процесс: имя клиента «para» в app() уникально, таблицы
// чистятся между кейсами (doctest выполняет кейсы последовательно).
struct Env
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    std::string configPath{(tmp.path / "config.json").string()};
    std::optional<ServerGuard> server;
    drogon::orm::DbClientPtr db;

    Env()
    {
        writeConfig();
        drogon::app().loadConfigFile(configPath);
        server.emplace();  // app().run() в потоке → createDbClients

        // Готовность: клиент «para» появляется при старте приложения
        // (DbClientManager::createDbClients из run()); HTTP не нужен.
        for (int i = 0; i < 500 && db == nullptr; ++i)  // ≤5 с
        {
            db = drogon::app().getDbClient("para");
            if (db == nullptr)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        REQUIRE(db != nullptr);

        dbExec("CREATE TABLE applications ("
               "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "  user_id INTEGER NOT NULL,"
               "  type TEXT NOT NULL,"
               "  status TEXT NOT NULL DEFAULT 'submitted'"
               "    CHECK (status IN ('draft','submitted','accepted',"
               "                      'in_progress','need_info','ready',"
               "                      'issued','rejected','cancelled')),"
               "  payload_json TEXT NOT NULL DEFAULT '{}',"
               "  created_at TEXT NOT NULL DEFAULT"
               "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')),"
               "  updated_at TEXT NOT NULL DEFAULT"
               "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')))");
        dbExec("CREATE TABLE application_events ("
               "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "  application_id INTEGER NOT NULL,"
               "  from_status TEXT,"
               "  to_status TEXT NOT NULL,"
               "  comment TEXT,"
               "  author_id INTEGER,"
               "  at TEXT NOT NULL DEFAULT"
               "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')))");
        dbExec("CREATE TABLE ics_tokens ("
               "  user_id    INTEGER PRIMARY KEY,"
               "  token      TEXT NOT NULL UNIQUE,"
               "  created_at TEXT NOT NULL,"
               "  revoked    INTEGER NOT NULL DEFAULT 0)");
    }

    void writeConfig()
    {
        const unsigned short port = findFreePort();
        REQUIRE(port != 0);
        Json::Value listener;
        listener["address"] = "127.0.0.1";
        listener["port"] = static_cast<Json::Value::Int>(port);
        listener["https"] = false;
        Json::Value log;
        log["use_spdlog"] = false;
        log["log_level"] = "WARN";
        Json::Value appCfg;
        appCfg["number_of_threads"] = 1;
        appCfg["enable_session"] = false;
        appCfg["log"] = log;
        Json::Value dbClient;
        dbClient["name"] = "para";
        dbClient["rdbms"] = "sqlite3";
        dbClient["filename"] = dbPath;
        dbClient["connection_number"] = 1;
        // FK=ON на каждом соединении пула — initialization_queries
        // прод-конфига (Э8 шаг 5); WAL/busy_timeout — как security_tests.
        Json::Value initQueries(Json::arrayValue);
        initQueries.append("PRAGMA journal_mode=WAL;");
        initQueries.append("PRAGMA synchronous=NORMAL;");
        initQueries.append("PRAGMA busy_timeout=5000;");
        initQueries.append("PRAGMA foreign_keys=ON;");
        dbClient["initialization_queries"] = initQueries;
        Json::Value root;
        root["listeners"].append(listener);
        root["app"] = appCfg;
        root["db_clients"].append(dbClient);

        Json::StreamWriterBuilder b;
        b["indentation"] = "  ";
        std::ofstream out(configPath, std::ios::binary | std::ios::trunc);
        REQUIRE(static_cast<bool>(out));
        out << Json::writeString(b, root);
        REQUIRE(static_cast<bool>(out));
    }

    // Один стейтмент через db-клиент (promise/future — колбэки приходят
    // с потока приложения; doctest-проверки — в потоке теста).
    void dbExec(const std::string& sql)
    {
        std::promise<bool> p;
        auto f = p.get_future();
        *db << sql >> [&p](const drogon::orm::Result&) { p.set_value(true); } >>
            [&p](const drogon::orm::DrogonDbException& e) {
                FAIL(e.base().what());
                p.set_value(false);
            };
        REQUIRE(f.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        REQUIRE(f.get());
    }

    // Кейсы последовательны (doctest), но порядок не гарантирован — каждый
    // начинается с чистых таблиц.
    void wipe()
    {
        dbExec("DELETE FROM applications");
        dbExec("DELETE FROM application_events");
        dbExec("DELETE FROM ics_tokens");
    }

    int64_t seedApp(int64_t userId,
                    const std::string& status,
                    const std::string& payload = "{}")
    {
        std::promise<int64_t> p;
        auto f = p.get_future();
        *db << "INSERT INTO applications (user_id, type, status, payload_json)"
               " VALUES (" +
                   std::to_string(userId) + ", 'study_certificate', '" +
                   status + "', '" + payload + "') RETURNING id" >>
            [&p](const drogon::orm::Result& rows) {
                p.set_value(rows.empty() ? 0
                                         : rows[0]["id"].as<int64_t>());
            } >>
            [&p](const drogon::orm::DrogonDbException& e) {
                FAIL(e.base().what());
                p.set_value(0);
            };
        REQUIRE(f.wait_for(std::chrono::seconds(10)) ==
                std::future_status::ready);
        const int64_t id = f.get();
        REQUIRE(id > 0);
        return id;
    }

    void seedToken(int64_t userId,
                   const std::string& token,
                   const std::string& createdAt,
                   int revoked)
    {
        dbExec("INSERT INTO ics_tokens (user_id, token, created_at, revoked)"
               " VALUES (" +
               std::to_string(userId) + ", '" + token + "', '" + createdAt +
               "', " + std::to_string(revoked) + ")");
    }
};

// Одна БД/клиент на процесс (имя «para» уникально в app()).
Env& env()
{
    static Env e;
    return e;
}

SessionInfo sessionOf(int64_t userId)
{
    if (userId == 4)
        return SessionInfo{4, "dina.demo", "student"};
    return SessionInfo{userId, "student" + std::to_string(userId), "student"};
}

// PUT /v1/applications/{id} напрямую в контроллер: атрибут «session»
// кладёт LoginFilter (в тесте — руками); body == nullopt — запрос без
// JSON-тела (getJsonObject == nullptr, ветка «guard ДО разбора тела»).
drogon::HttpResponsePtr putUpdate(int64_t userId,
                                  int64_t appId,
                                  std::optional<Json::Value> body)
{
    ApplicationsCtrl ctrl;
    drogon::HttpRequestPtr req =
        body ? drogon::HttpRequest::newHttpJsonRequest(*body)
             : drogon::HttpRequest::newHttpRequest();
    req->attributes()->insert("session", sessionOf(userId));
    std::promise<drogon::HttpResponsePtr> p;
    auto f = p.get_future();
    ctrl.updateDraft(
        req,
        [&p](const drogon::HttpResponsePtr& r) { p.set_value(r); },
        appId);
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// GET /v1/ics/token напрямую в контроллер (LoginFilter-атрибут — руками).
drogon::HttpResponsePtr getIcsStatus(int64_t userId)
{
    IcsCtrl ctrl;
    drogon::HttpRequestPtr req = drogon::HttpRequest::newHttpRequest();
    req->attributes()->insert("session", sessionOf(userId));
    std::promise<drogon::HttpResponsePtr> p;
    auto f = p.get_future();
    ctrl.tokenStatus(
        req, [&p](const drogon::HttpResponsePtr& r) { p.set_value(r); });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

// Карточка + таймлайн через сервис (независимый от контроллера путь).
ApplicationsService::OneResult oneOf(int64_t appId)
{
    std::promise<ApplicationsService::OneResult> p;
    auto f = p.get_future();
    ApplicationsService::one(
        env().db, appId, [&p](ApplicationsService::OneResult r) {
            p.set_value(std::move(r));
        });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

const Json::Value& jsonOf(const drogon::HttpResponsePtr& resp)
{
    const auto json = resp->getJsonObject();
    REQUIRE(json != nullptr);
    return *json;
}

Json::Value submitBody(const std::string& payloadJson)
{
    Json::Value body;
    body["payload_json"] = payloadJson;
    body["submit"] = true;
    return body;
}

}  // namespace

TEST_CASE("ApplicationsCtrl::updateDraft submit:true — своя draft: 200 "
          "submitted + событие подачи с датой (8.i)")
{
    Env& e = env();
    e.wipe();
    const int64_t draft = e.seedApp(4, "draft", "{\"copies\":1}");

    const auto resp = putUpdate(4, draft, submitBody(
        "{\"copies\":2,\"delivery\":\"office\"}"));
    REQUIRE(resp != nullptr);
    CHECK(resp->getStatusCode() == drogon::k200OK);
    const auto& body = jsonOf(resp);
    CHECK(body["status"].asString() == "submitted");
    CHECK(body["payload"]["copies"].asInt64() == 2);
    CHECK(body["payload"]["delivery"].asString() == "office");

    // Событие подачи — через сервис (таймлайн): draft→submitted, дата
    // «at» — формат DDL 20 символов UTC, автор — владелец сессии.
    const auto card = oneOf(draft);
    REQUIRE(card.status == ApplicationsService::One::Found);
    CHECK(card.body["status"].asString() == "submitted");
    REQUIRE(card.body["timeline"].size() == 1);
    const auto& ev = card.body["timeline"][0];
    CHECK(ev["application_id"].asInt64() == draft);
    CHECK(ev["from_status"].asString() == "draft");
    CHECK(ev["to_status"].asString() == "submitted");
    CHECK(ev["author_id"].asInt64() == 4);
    CHECK(ev["at"].asString().size() == 20);  // %Y-%m-%dT%H:%M:%SZ
    CHECK(ev["at"].asString().back() == 'Z');
}

TEST_CASE("ApplicationsCtrl::updateDraft submit:true — не-draft: 409 "
          "invalid_status_transition, payload не тронут")
{
    Env& e = env();
    e.wipe();
    const int64_t submitted = e.seedApp(4, "submitted", "{\"copies\":1}");
    const int64_t issued = e.seedApp(4, "issued", "{\"copies\":1}");

    // Подача уже поданной → 409-контракт 02 §5.1/§5.2 (Р-29).
    const auto resp = putUpdate(4, submitted, submitBody("{\"copies\":9}"));
    REQUIRE(resp != nullptr);
    CHECK(resp->getStatusCode() == drogon::k409Conflict);
    const auto& err = jsonOf(resp)["error"];
    CHECK(err["code"].asString() == "invalid_status_transition");
    CHECK(err["message"].asString() == "Подать можно только черновик заявки");

    // Терминальная — тот же контракт.
    const auto terminal = putUpdate(4, issued, submitBody("{\"copies\":9}"));
    REQUIRE(terminal != nullptr);
    CHECK(terminal->getStatusCode() == drogon::k409Conflict);
    CHECK(jsonOf(terminal)["error"]["code"].asString() ==
          "invalid_status_transition");

    // Ничего не изменилось: payload прежний, событий подачи не появилось.
    for (const int64_t id : {submitted, issued})
    {
        const auto card = oneOf(id);
        REQUIRE(card.status == ApplicationsService::One::Found);
        CHECK(card.body["payload"]["copies"].asInt64() == 1);
        CHECK(card.body["timeline"].size() == 0);
    }
}

TEST_CASE("ApplicationsCtrl::updateDraft — guard ДО тела и статуса: чужая "
          "→ 403 forbidden, несуществующая → 404")
{
    Env& e = env();
    e.wipe();
    // Чужой ЧЕРНОВИК (владелец 2, сессия 4): без guard это была бы подача
    // 200 — контракт «guard до статуса» даёт 403, код forbidden (не 409).
    const int64_t foreign = e.seedApp(2, "draft", "{\"copies\":1}");

    const auto resp = putUpdate(4, foreign, submitBody("{\"copies\":9}"));
    REQUIRE(resp != nullptr);
    CHECK(resp->getStatusCode() == drogon::k403Forbidden);
    const auto& err = jsonOf(resp)["error"];
    CHECK(err["code"].asString() == "forbidden");
    CHECK(err["message"].asString() == "Доступ только к своим данным");

    // Guard ДО разбора тела: чужая заявка с запросом БЕЗ JSON-тела —
    // 403 (не 400 «Ожидается JSON-тело»).
    const auto noBody = putUpdate(4, foreign, std::nullopt);
    REQUIRE(noBody != nullptr);
    CHECK(noBody->getStatusCode() == drogon::k403Forbidden);
    CHECK(jsonOf(noBody)["error"]["code"].asString() == "forbidden");

    // Чужая не-draft — тоже 403 (не маскируется под 409).
    const int64_t foreignSubmitted = e.seedApp(2, "submitted");
    const auto foreign409 = putUpdate(4, foreignSubmitted,
                                      submitBody("{\"copies\":9}"));
    REQUIRE(foreign409 != nullptr);
    CHECK(foreign409->getStatusCode() == drogon::k403Forbidden);

    // Несуществующая → 404 (не 403 — не раскрываем существование).
    const auto missing = putUpdate(4, 999999, submitBody("{\"copies\":9}"));
    REQUIRE(missing != nullptr);
    CHECK(missing->getStatusCode() == drogon::k404NotFound);
    CHECK(jsonOf(missing)["error"]["code"].asString() == "not_found");

    // Попытки ничего не изменили: чужой черновик остался draft с прежним
    // payload, событий нет.
    const auto card = oneOf(foreign);
    REQUIRE(card.status == ApplicationsService::One::Found);
    CHECK(card.body["status"].asString() == "draft");
    CHECK(card.body["payload"]["copies"].asInt64() == 1);
    CHECK(card.body["timeline"].size() == 0);
}

TEST_CASE("IcsCtrl::tokenStatus — активный 200 {token, created_at}; нет "
          "→ 404; отозван → 404 (8.ii)")
{
    Env& e = env();
    e.wipe();

    // Нет строки — «нет подписки» → 404 not_found (02 §5.1).
    const auto none = getIcsStatus(4);
    REQUIRE(none != nullptr);
    CHECK(none->getStatusCode() == drogon::k404NotFound);
    const auto& err = jsonOf(none)["error"];
    CHECK(err["code"].asString() == "not_found");
    CHECK(err["message"].asString() == "Подписка не найдена");

    // Активный токен → 200: ровно {token, created_at} — те же значения,
    // что в приватной ссылке /ics/{token}.
    const std::string token(64, 'a');  // формат Р-11: 64 hex
    e.seedToken(4, token, "2026-11-13T09:15:00Z", 0);
    const auto active = getIcsStatus(4);
    REQUIRE(active != nullptr);
    CHECK(active->getStatusCode() == drogon::k200OK);
    const auto& body = jsonOf(active);
    REQUIRE(body.isObject());
    CHECK(body.getMemberNames().size() == 2);  // только token + created_at
    CHECK(body["token"].asString() == token);
    CHECK(body["created_at"].asString() == "2026-11-13T09:15:00Z");

    // Отозван (revoked=1) → снова 404 — сам токен не раскрывается.
    e.dbExec("UPDATE ics_tokens SET revoked = 1 WHERE user_id = 4");
    const auto revoked = getIcsStatus(4);
    REQUIRE(revoked != nullptr);
    CHECK(revoked->getStatusCode() == drogon::k404NotFound);
    CHECK(jsonOf(revoked)["error"]["code"].asString() == "not_found");
}
