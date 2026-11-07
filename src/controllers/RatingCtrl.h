#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Рейтинг факультета (Э6а, 5.14; 02 §5.2: GET /v1/rating — query faculty,
// limit). Тонкий контроллер (AGENTS.md §5.5): SQL и маскирование —
// RatingService; роут под LoginFilter — без cookie 401 (02 §6.3).
//
// query faculty — применяется ТОЛЬКО от staff (рейтинг по факультету);
// студент всегда видит рейтинг СВОЕГО факультета (факультет из БД по
// сессии — 02 §6.3, query не подменяет — Р-15-стиль). query limit —
// опционален; нечисловой/вне 1..1000 → 400 (стиль days в ScheduleCtrl).
//
// МАСКИРОВАНИЕ НА СЕРВЕРЕ (02 §5.2): полное имя — только в строке самого
// user_id сессии; чужие строки — «И. Фамилия»; полное имя чужих из БД
// наружу не уходит вовсе. Path-id нет — OwnershipGuard не применяется.
class RatingCtrl : public HttpController<RatingCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RatingCtrl::show, "/v1/rating", Get, "LoginFilter");
    METHOD_LIST_END

    void show(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
};
