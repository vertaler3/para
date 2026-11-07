#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Дашборд «Сейчас важно» (экран «Сегодня», Э7 FE; 03 §4.2, 02 §5.2 GET
// /v1/dashboard). Тонкий контроллер (AGENTS §3 «10–30 строк», §5.5;
// правки по ревью экрана — агрегация перенесена в DashboardService):
//   GET /              — SSR-страница «Сегодня» (views/dashboard.csp).
//       Путь «/» в белом списке LoginFilter (публичный, LoginFilter.cpp:
//       isPublic → path == "/"), поэтому сессию проверяет САМ контроллер:
//       cookie → SessionService::findValid; сессии нет → redirect /login
//       (k303SeeOther — 02 §6.3), есть → newHttpViewResponse("dashboard")
//       с activeTab="today" и pageTitle="Сегодня · Пара".
//   GET /v1/dashboard  — JSON-агрегат: DashboardService::dashboard()
//       (публичные API существующих сервисов, SQL в контроллере нет);
//       ok → 200, иначе 500 (fail-closed). path-id нет — OwnershipGuard
//       не применим (правило — для эндпоинтов с {id} в пути).
// Контракт ответа агрегата — в шапке DashboardService.h.
class DashboardCtrl : public HttpController<DashboardCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DashboardCtrl::page, "/", Get, "LoginFilter");
    ADD_METHOD_TO(DashboardCtrl::aggregate, "/v1/dashboard", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void page(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void aggregate(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
};
