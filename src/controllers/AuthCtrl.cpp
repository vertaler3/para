#include "controllers/AuthCtrl.h"

#include <chrono>

#include "filters/CsrfCheck.h"
#include "services/PasswordHasher.h"
#include "services/UserService.h"
#include "utils/ApiError.h"

using namespace para;
using namespace drogon::orm;

namespace {

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
    // CSRF double-submit — с шага 2 Э8 проверяется CsrfCheck на всех
    // приватных state-changing (вызов из LoginFilter, 02 §6.4). Сам вход —
    // до сессии, токена ещё не существует: вне CsrfCheck (журнал Э8 шага 2;
    // AGENTS §4 смок сохранён), барьеры — SameSite=Lax cookie и лимитер
    // ниже.
    const auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("login") ||
        !json->isMember("password"))
    {
        cb(api::errorResponse("bad_request",
                              "Ожидается JSON с полями login и password",
                              k400BadRequest));
        return;
    }
    const std::string login = (*json)["login"].asString();
    const std::string password = (*json)["password"].asString();

    // Лимитер (02 §6.4): заблокирован → 429, попытка не проверяется.
    if (const int64_t wait = limiter_.blockedFor(login, nowSec()); wait > 0)
    {
        auto resp = api::errorResponse("too_many_attempts",
                                       "Слишком много попыток входа, "
                                       "повторите позже",
                                       k429TooManyRequests);
        resp->addHeader("Retry-After", std::to_string(wait));
        cb(resp);
        return;
    }

    UserService::findByLogin(
        app().getDbClient("para"), login,
        [this, login, password,
         cb = std::move(cb)](std::optional<UserRecord> user) mutable {
            const bool passwordOk =
                user.has_value() &&
                PasswordHasher::verify(user->passwordHash, password);
            if (!passwordOk)
            {
                // 401 без деталей (01, экран «Вход»): не раскрываем, что
                // именно неверно; неудача учитывается и для несуществующего
                // логина.
                limiter_.onFailure(login, nowSec());
                UserService::mirrorFailedAttempts(
                    app().getDbClient("para"), login,
                    limiter_.failures(login));
                // Единый message для неверного логина и неверного пароля —
                // без деталей, что именно не подошло (02 §5.2).
                cb(api::errorResponse("invalid_credentials",
                                      "Неверный логин или пароль",
                                      k401Unauthorized));
                return;
            }
            limiter_.onSuccess(login);
            const int64_t userId = user->id;
            const std::string role = user->role;
            const int64_t ttl = SessionService::currentTtlSec();
            SessionService::create(
                app().getDbClient("para"), userId, ttl,
                [login = std::move(login), role, ttl,
                 cb = std::move(cb)](bool ok, const std::string& token) {
                    if (!ok)
                    {
                        // 500 без стека и служебных строк наружу
                        // (02 §5.1: детали — только в серверном логе).
                        cb(api::errorResponse("internal",
                                              "Внутренняя ошибка сервера",
                                              k500InternalServerError));
                        return;
                    }
                    Json::Value body;
                    body["login"] = login;
                    body["role"] = role;
                    auto resp = jsonResponse(std::move(body), k200OK);
                    resp->addHeader("Set-Cookie",
                                    SessionService::sessionCookie(token, ttl));
                    // CSRF double-submit (02 §6.4, Э8 шаг 2): вместе с
                    // сессией выдаём cookie csrf (привязан к этой сессии,
                    // без HttpOnly — токен читает app.js; CsrfCheck.h) —
                    // все state-changing после входа требуют токена.
                    resp->addCookie(CsrfCheck::cookieFor(token, ttl));
                    cb(resp);
                });
        });
}

void AuthCtrl::logout(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Инвалидация на сервере: DELETE FROM sessions — в SessionService
    // (02 §6.1). Тонкий: достал cookie, передал в сервис.
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
    UserService::loadProfile(
        app().getDbClient("para"), session.userId,
        [session, cb = std::move(cb)](std::optional<Json::Value> profile) {
            Json::Value body;
            body["user_id"] = session.userId;
            body["login"] = session.login;
            body["role"] = session.role;
            if (profile.has_value())
            {
                body["profile"] = std::move(*profile);
            }
            // У staff (dean.demo) профиля нет — базовые поля без profile.
            cb(jsonResponse(std::move(body), k200OK));
        });
}
