#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Библиотека (5.12; Э6б шаг 2; 02 §5.2). Тонкий контроллер (AGENTS.md §5.5):
// SQL и фильтры — LibraryService; роуты под LoginFilter — без cookie 401 от
// фильтра (02 §6.3). Каталоги филиалов/книг — общий ресурс (OwnershipGuard
// не применяется — как аудитории Э5а, шапка LibraryService); личный объект —
// читательский билет, выбирается строго по user_id сессии (path-id нет).
//
//   GET /v1/library/branches  — филиалы с корпусом → {"branches": [...]};
//   GET /v1/library/items     — наличие литературы → {"items": [...]},
//       query 02 §5.2: ?q=подстрока (title/author), ?branch_id=N (не целое
//       → 400, стиль capacity в RoomsCtrl), ?available=1 — только
//       available > 0 («свободно сейчас»); иное значение available → 400
//       (стиль upcoming в RoomsCtrl — известные значения только из
//       контракта); сортировка по title;
//   GET /v1/library/my-card   — билет ТЕКУЩЕГО пользователя (данные для
//       QR: card_no + ФИО/группа из своего профиля, 5.12); нет билета →
//       404 not_found (02 §5.2 «404 (нет билета)»).
//
// Ошибки — единый формат 02 §5.1 (api::errorResponse); успешные ответы —
// api::jsonResponse (кириллица сырыми UTF-8 байтами, гейт-конвенция Э6 р1).
//
// Страничная плоскость (Э7, экран «Библиотека» 03 §4.9.6/5.12; Р-19 —
// путь вне /v1/ по постановке экрана):
//   GET /library           — SSR-страница views/library.csp: строка поиска,
//       чипы филиалов (все + по одному), чип «Только доступные», строки
//       книг с бейджем «свободно N из M». Без сессии LoginFilter отвечает
//       303 → /login (страница). Данных страница не содержит — филиалы и
//       наличие грузит клиент из ТЕХ ЖЕ JSON-роутов выше (GET
//       /v1/library/branches, GET /v1/library/items — 02 §5.2; паттерн
//       RoomsCtrl::roomsPage); читательский билет — на /profile (5.9/5.12).
class LibraryCtrl : public HttpController<LibraryCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(LibraryCtrl::getBranches, "/v1/library/branches", Get,
                  "LoginFilter");
    ADD_METHOD_TO(LibraryCtrl::getItems, "/v1/library/items", Get,
                  "LoginFilter");
    ADD_METHOD_TO(LibraryCtrl::getMyCard, "/v1/library/my-card", Get,
                  "LoginFilter");
    ADD_METHOD_TO(LibraryCtrl::libraryPage, "/library", Get, "LoginFilter");
    METHOD_LIST_END

    void getBranches(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
    void getItems(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void getMyCard(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void libraryPage(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
};
