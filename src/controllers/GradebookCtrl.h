#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Зачётная книжка (Э6а, 5.15; 02 §5.2: GET /v1/gradebook — «дисциплины с
// итогами, средний балл, счётчики»). Тонкий контроллер (AGENTS.md §5.5):
// SQL и арифметика — GradebookService; роут под LoginFilter — без cookie
// 401 от фильтра (02 §6.3). user_id — только из сессии (02 §6.3): книжка
// всегда СВОЯ, path-id нет — OwnershipGuard не применяется (правило
// код-ревью касается path-id; решение Р-10: расчёт без новых сущностей).
class GradebookCtrl : public HttpController<GradebookCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GradebookCtrl::show, "/v1/gradebook", Get, "LoginFilter");
    METHOD_LIST_END

    void show(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
};
