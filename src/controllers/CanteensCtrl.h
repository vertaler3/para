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
class CanteensCtrl : public HttpController<CanteensCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CanteensCtrl::getCanteens, "/v1/canteens", Get,
                  "LoginFilter");
    ADD_METHOD_TO(CanteensCtrl::getMenu, "/v1/canteens/{id}/menu", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void getCanteens(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
    void getMenu(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb,
                 int64_t canteenId);
};
