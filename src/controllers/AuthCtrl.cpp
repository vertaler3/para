#include "controllers/AuthCtrl.h"

#include <chrono>

#include "services/PasswordHasher.h"

using namespace para;
using namespace drogon::orm;

namespace {

Json::String errorBody(const char* error)
{
    Json::Value body;
    body["error"] = error;
    return body.toStyledString();
}

HttpResponsePtr jsonResponse(Json::Value body, HttpStatusCode code)
{
    auto resp = HttpResponse::newHttpJsonResponse(std::move(body));
    resp->setStatusCode(code);
    return resp;
}

int64_t nowSec()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

}  // namespace

void AuthCtrl::loginPage(const HttpRequestPtr&,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Тонкий: только рендер CSP-шаблона views/login.csp (03 §4.1; стили —
    // FE-дорожка). Скомпилированный шаблон — DrTemplate «login».
    cb(HttpResponse::newHttpViewResponse("login"));
}

void AuthCtrl::login(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb)
{
    // CSRF-проверка на state-changing — Э8 (02 §6.4: double-submit +
    // HX-Request); сейчас барьер — SameSite=Lax у cookie.
    const auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("login") ||
        !json->isMember("password"))
    {
        cb(jsonResponse(errorBody("bad_request"), k400BadRequest));
        return;
    }
    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    // Лимитер (02 §6.4): заблокирован → 429, попытка не проверяется.
    if (const int64_t wait = limiter_.blockedFor(login, nowSec()); wait > 0)
    {
        auto resp = jsonResponse(errorBody("too_many_attempts"),
                                 k429TooManyRequests);
        resp->addHeader("Retry-After", std::to_string(wait));
        cb(resp);
        return;
    }

    auto db = app().getDbClient("para");
    *db << "SELECT id, login, role, password_hash FROM users WHERE login = $1"
        << login >>
        [this, login, password, cb = std::move(cb)](const Result& rows) mutable {
            const bool userFound = !rows.empty();
            const bool passwordOk =
                userFound &&
                PasswordHasher::verify(
                    rows[0]["password_hash"].as<std::string>(), password);
            if (!passwordOk)
            {
                // 401 без деталей (01, С-«Вход»): не раскрываем, что именно
                // неверно; неудача учитывается и для несуществующего логина.
                limiter_.onFailure(login, nowSec());
                const int fails = limiter_.failures(login);
                // Зеркало состояния в users (02 §4.1/§6.4): best-effort.
                *app().getDbClient("para")
                    << "UPDATE users SET failed_attempts = $1 WHERE login = $2"
                    << fails << login >> [](const Result&) {} >>
                    [](const DrogonDbException& e) {
                        LOG_ERROR << "limiter mirror: " << e.base().what();
                    };
                cb(jsonResponse(errorBody("invalid_credentials"),
                                k401Unauthorized));
                return;
            }
            limiter_.onSuccess(login);
            const int64_t userId = rows[0]["id"].as<int64_t>();
            const std::string role = rows[0]["role"].as<std::string>();
            const int64_t ttl = SessionService::currentTtlSec();
            SessionService::create(
                app().getDbClient("para"),
                userId,
                ttl,
                [login = std::move(login), role, ttl,
                 cb = std::move(cb)](bool ok, const std::string& token) {
                    if (!ok)
                    {
                        cb(jsonResponse(errorBody("internal"),
                                        k500InternalServerError));
                        return;
                    }
                    Json::Value body;
                    body["login"] = login;
                    body["role"] = role;
                    auto resp = jsonResponse(std::move(body), k200OK);
                    resp->addHeader("Set-Cookie",
                                    SessionService::sessionCookie(token, ttl));
                    cb(resp);
                });
        } >>
        [cb = std::move(cb)](const DrogonDbException& e) {
            LOG_ERROR << "AuthCtrl::login: " << e.base().what();
            cb(jsonResponse(errorBody("internal"), k500InternalServerError));
        };
}

void AuthCtrl::logout(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Инвалидация на сервере: DELETE FROM sessions (02 §6.1).
    const std::string token = req->getCookie(SessionService::kCookieName);
    SessionService::remove(app().getDbClient("para"), token,
                           [cb = std::move(cb)]() {
                               Json::Value body;
                               body["ok"] = true;
                               cb(jsonResponse(std::move(body), k200OK));
                           });
}

void AuthCtrl::me(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию уже проверил LoginFilter — данные в attributes.
    const auto session =
        req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    *db << "SELECT p.fio, p.faculty, p.program, p.course, g.name AS group_name, "
           "p.student_card_no FROM profiles p JOIN groups g ON g.id = p.group_id "
           "WHERE p.user_id = $1"
        << session.userId >>
        [session, cb = std::move(cb)](const Result& rows) {
            Json::Value body;
            body["user_id"] = session.userId;
            body["login"] = session.login;
            body["role"] = session.role;
            if (!rows.empty())
            {
                const auto& row = rows[0];
                body["profile"]["fio"] = row["fio"].as<std::string>();
                body["profile"]["faculty"] = row["faculty"].as<std::string>();
                body["profile"]["program"] = row["program"].as<std::string>();
                body["profile"]["course"] = row["course"].as<int>();
                body["profile"]["group"] = row["group_name"].as<std::string>();
                body["profile"]["student_card_no"] =
                    row["student_card_no"].as<std::string>();
            }
            // У staff (dean.demo) профиля нет — базовые поля без profile.
            cb(jsonResponse(std::move(body), k200OK));
        } >>
        [cb = std::move(cb)](const DrogonDbException& e) {
            LOG_ERROR << "AuthCtrl::me: " << e.base().what();
            cb(jsonResponse(errorBody("internal"), k500InternalServerError));
        };
}
