#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Новости и события (фича 5.17, таблица news миграции 0007). Тонкий
// контроллер (AGENTS.md §5.5): парсинг query, маппинг исходов сервиса в
// HTTP; SQL и правила — NewsService. Роут под LoginFilter — без cookie
// 401 от фильтра (02 §6.3), CSRF на POST не нужен (GET-чтение); публикация
// идёт через существующий POST /v1/staff/announcements (kind='news',
// StaffCtrl) — НОВОГО staff-роута нет.
//
//   GET /v1/news — лента, видимая текущей сессии ('all' — всем; 'group' —
//       своей группе, 'course' — своему курсу; группа/курс — из профиля
//       сессии; staff без профиля видит все, прецедент объявлений).
//       query: ?category= official|conference|org|event (иное значение
//       → 400 — словарь CHECK 0007 проверяет сервис), ?cursor= id
//       последнего загруженного (нечисловой/<= 0 → 400 в контроллере;
//       чужой по аудитории/несуществующий → 400 в сервисе).
//       Сортировка published_at DESC; составной курсор (published_at, id) —
//       контракт NotificationsFeedService: LIMIT 20+1, next_cursor = id
//       последней строки, только если есть 21-я.
//
// Роут регистрируется METHOD_LIST (авторегистрация Drogon при линковке
// в цель — как AnnouncementsCtrl/NotificationsFeedCtrl; в main.cpp
// контроллеры не перечислены).
class NewsCtrl : public HttpController<NewsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(NewsCtrl::feed, "/v1/news", Get, "LoginFilter");
    METHOD_LIST_END

    void feed(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
};
