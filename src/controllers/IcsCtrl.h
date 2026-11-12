#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Приватная подписка календаря (Э3 шаг 2; идея 5.4, 02 §5.2):
//   GET    /ics/{token}    — ПУБЛИЧНЫЙ: авторизация самим секретным токеном
//                            (02 §6.1: «/ics/{token} — без сессионной cookie»);
//                            LoginFilter пропускает префикс /ics/ без сессии;
//   GET    /v1/ics/token   — статус подписки: активный токен + created_at
//                            (по сессии, 200; нет/отозван — 404; 8.ii);
//   POST   /v1/ics/token   — создать/перевыпустить токен (по сессии, 201);
//   DELETE /v1/ics/token   — отозвать (по сессии, 204).
// Тонкий контроллер (AGENTS.md §5.5): SQL и сборка VCALENDAR — IcsService/
// IcsBuilder; path-id {token} — владение не из сессии, а по токену
// (OwnershipGuard не применим: объект ищется не по user_id сессии).
class IcsCtrl : public HttpController<IcsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(IcsCtrl::getCalendar, "/ics/{token}", Get);
    ADD_METHOD_TO(IcsCtrl::tokenStatus, "/v1/ics/token", Get, "LoginFilter");
    ADD_METHOD_TO(IcsCtrl::createToken, "/v1/ics/token", Post, "LoginFilter");
    ADD_METHOD_TO(IcsCtrl::revokeToken, "/v1/ics/token", Delete, "LoginFilter");
    METHOD_LIST_END

    void getCalendar(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb,
                     std::string token);
    void tokenStatus(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
    void createToken(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
    void revokeToken(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
};
