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
//
// Страничная плоскость (Э7, экран «Рейтинг»; путь вне /v1/ — по
// постановке 02 §3.2, как /schedule — Р-19):
//   GET /rating — views/rating.csp (03 §4.9.6 «Рейтинг 5.14», черновик
//       design-drafts/ranking.html). Тело страницы — ТОТ ЖЕ метод
//       RatingService::rating, что стоит за JSON API (SQL в контроллере
//       нет, AGENTS §5.5); query faculty/limit страница НЕ принимает —
//       общий рейтинг (дефолт API, уточнение Р-15: рейтинг общий, чужие
//       маскированы) без обрезки, валидация limit остаётся делом
//       JSON-роута. Свою строку шаблон различает по флагу self из тела
//       сервиса (RatingService.h:61-63) — сопоставление с /v1/me на
//       клиенте не нужно. OwnershipGuard не применяется — path-id нет;
//       ошибка БД → 500 fail-closed от JSON-роута, страница при том же
//       сбое рендерит состояние ошибки (failed=1, 03 §5.3 — паттерн
//       GradesCtrl::gradesPage). Без сессии LoginFilter отвечает
//       303 → /login (страница, не /v1/ — 02 §6.3).
class RatingCtrl : public HttpController<RatingCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RatingCtrl::show, "/v1/rating", Get, "LoginFilter");
    ADD_METHOD_TO(RatingCtrl::ratingPage, "/rating", Get, "LoginFilter");
    METHOD_LIST_END

    void show(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void ratingPage(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
};
