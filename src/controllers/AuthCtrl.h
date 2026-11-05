#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"
#include "utils/LoginLimiter.h"

using namespace drogon;

// Тонкий контроллер входа (10–30 строк на эндпоинт, AGENTS.md §5.5):
// логика — в сервисах (PasswordHasher, SessionService, LoginLimiter),
// SQL — параметризованный, только в сервисах.
// /v1/auth/login — публичный; /v1/auth/logout и /v1/me — под LoginFilter.
// GET /v1/me — минимальный профиль для смока Э1; полное развитие — Э2.
class AuthCtrl : public HttpController<AuthCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthCtrl::loginPage, "/login", Get);
    ADD_METHOD_TO(AuthCtrl::login, "/v1/auth/login", Post);
    ADD_METHOD_TO(AuthCtrl::logout, "/v1/auth/logout", Post, "LoginFilter");
    ADD_METHOD_TO(AuthCtrl::me, "/v1/me", Get, "LoginFilter");
    METHOD_LIST_END

    // Страница входа: рендер CSP-шаблона views/login.csp (скомпилирован в
    // DrTemplate «login» — сам роут не создаёт, поэтому отдаёт контроллер).

    void loginPage(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void login(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& cb);
    void logout(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb);
    void me(const HttpRequestPtr& req,
            std::function<void(const HttpResponsePtr&)>&& cb);

  private:
    // Один на процесс: состояние лимитера в памяти (02 §6.4, Р-7).
    para::LoginLimiter limiter_;
};
