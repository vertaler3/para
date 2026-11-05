#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Успеваемость (Э4 шаг 1; 02 §5.2: GET /v1/grades — «свои оценки по
// дисциплинам», GET /v1/grades/summary — «сводка семестра»). Тонкий
// контроллер (AGENTS.md §5.5): SQL и логика — GradesService; оба маршрута
// под LoginFilter — без cookie 401 от фильтра (02 §6.3). user_id — только
// из сессии: path-id нет, OwnershipGuard не применим; GET /v1/grades/{id}
// (владение через guard, 02 §5.2) — отдельный шаг Э4, здесь не закладывается.
class GradesCtrl : public HttpController<GradesCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GradesCtrl::getGrades, "/v1/grades", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getSummary, "/v1/grades/summary", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void getGrades(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void getSummary(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
};
