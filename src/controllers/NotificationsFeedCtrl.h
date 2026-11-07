#pragma once

#include <drogon/HttpController.h>

#include "services/OwnershipGuard.h"  // Table::Notifications — guard для {id}/read
#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Лента уведомлений (Э6, закрытие пробела; 02 §5.2). Тонкий контроллер
// (AGENTS.md §5.5): SQL и правила — NotificationsFeedService; роуты под
// LoginFilter — без cookie 401 от фильтра (02 §6.3). user_id — только из
// сессии.
//
//   GET  /v1/notifications             — плоская лента (новые сверху):
//       query 02 §5.2 ?unread=1 (иное значение → 400, стиль upcoming),
//       ?type= (словарь CHECK 0004, иное → 400), ?cursor= (id последнего
//       загруженного; нечисловой/чужой/несуществующий → 400);
//   POST /v1/notifications/{id}/read — отметить прочитанным → 204.
//       ПРАВИЛО КОД-РЁВЬЮ (OwnershipGuard.h): path-id ЛИЧНОГО объекта →
//       OwnershipGuard::check(Table::Notifications) ДО любых данных
//       (02 §5.2: 204 · 403 · 404 — чужое существующее → 403 «Доступ
//       только к своим данным» ДО пометки, несуществующее → 404,
//       ошибка БД → 500 fail-closed); повторный read уже прочитанного —
//       тот же 204 (идемпотентность сервиса);
//   POST /v1/notifications/read-all  — пометить всё прочитанным → 204.
//
// Роуты не конфликтуют: read-all/settings — статические сегменты (3-й
// уровень), {id}/read — 4-й уровень; статический приоритетнее (сверено
// E2E в Э4 для /grades/summary).
//
// Группировка по типам (02 §5.2) — опция ОТОБРАЖЕНИЯ экрана 03 §4.6
// (колокольчик/лента), задача FE-дорожки; API отдаёт плоскую ленту —
// зафиксировано в журнале сессии.
//
// markRead/markAllRead уведомлений НЕ создают и push не стреляют — только
// UPDATE read_at (строки ленты живут в notifications, источник истины).
class NotificationsFeedCtrl : public HttpController<NotificationsFeedCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(NotificationsFeedCtrl::feed, "/v1/notifications", Get,
                  "LoginFilter");
    ADD_METHOD_TO(NotificationsFeedCtrl::markRead,
                  "/v1/notifications/{id}/read", Post, "LoginFilter");
    ADD_METHOD_TO(NotificationsFeedCtrl::markAllRead,
                  "/v1/notifications/read-all", Post, "LoginFilter");
    METHOD_LIST_END

    void feed(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void markRead(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t notificationId);
    void markAllRead(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb);
};
