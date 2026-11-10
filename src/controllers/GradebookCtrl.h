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
//
// Страничная плоскость (Э7, экран «Зачётная книжка»; путь вне /v1/ — по
// постановке 02 §3.2, как /schedule — Р-19):
//   GET /gradebook — views/gradebook.csp (03 §4.9.6 «Зачётная книжка
//       5.15», черновик design-drafts/recordbook.html). Тело страницы —
//       ТОТ ЖЕ метод GradebookService::gradebook, что стоит за JSON API
//       (SQL в контроллере нет, AGENTS §5.5); OwnershipGuard не
//       применяется — path-id нет, книжка всегда своя (user_id сессии,
//       02 §6.3). Ошибка БД → 500 fail-closed от JSON-роута, страница
//       при том же сбое рендерит состояние ошибки (failed=1, 03 §5.3 —
//       паттерн GradesCtrl::gradesPage). Без сессии LoginFilter отвечает
//       303 → /login (страница, не /v1/ — 02 §6.3).
class GradebookCtrl : public HttpController<GradebookCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GradebookCtrl::show, "/v1/gradebook", Get, "LoginFilter");
    ADD_METHOD_TO(GradebookCtrl::gradebookPage, "/gradebook", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void show(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void gradebookPage(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb);
};
