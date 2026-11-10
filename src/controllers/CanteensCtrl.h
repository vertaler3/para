#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Столовые (5.13; Э6б шаг 3; 02 §5.2). Тонкий контроллер (AGENTS.md §5.5):
// SQL — CanteensService; роуты под LoginFilter — без cookie 401 от фильтра
// (02 §6.3). Каталог общий, user-скоупа нет; {id} — публичная столовая:
// несуществующая → 404 not_found (02 §5.2 «200 · 404»).
//
//   GET /v1/canteens        — список: корпус, часы → {"canteens": [...]};
//   GET /v1/canteens/{id}/menu — меню: цены, комплексные, КБЖУ →
//       {"canteen_id", "canteen", "building", "hours", "menu": [...]};
//       столовой нет → 404.
//
// Ошибки — единый формат 02 §5.1 (api::errorResponse); успешные —
// api::jsonResponse (кириллица сырыми UTF-8 байтами, гейт-конвенция Э6 р1).
//
// Страничная плоскость (Э7, экран «Столовые» 03 §4.9.6/5.13; Р-19 —
// путь вне /v1/ по постановке экрана):
//   GET /canteens          — SSR-страница views/canteens.csp: чипы столовых
//       (название · корпус · часы) и меню выбранной (комбо раскрывается,
//       цены справа tnum, КБЖУ строкой caption). Без сессии LoginFilter
//       отвечает 303 → /login (страница). Данных страница не содержит —
//       список столовых и меню грузит клиент из ТЕХ ЖЕ JSON-роутов выше
//       (GET /v1/canteens, GET /v1/canteens/{id}/menu — 02 §5.2; паттерн
//       RoomsCtrl::roomsPage).
class CanteensCtrl : public HttpController<CanteensCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CanteensCtrl::getCanteens, "/v1/canteens", Get,
                  "LoginFilter");
    ADD_METHOD_TO(CanteensCtrl::getMenu, "/v1/canteens/{id}/menu", Get,
                  "LoginFilter");
    ADD_METHOD_TO(CanteensCtrl::canteensPage, "/canteens", Get, "LoginFilter");
    METHOD_LIST_END

    void getCanteens(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
    void getMenu(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb,
                 int64_t canteenId);
    void canteensPage(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb);
};
