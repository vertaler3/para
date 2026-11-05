#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Настройки уведомлений (Э2 шаг 3; 02 §5.2 GET/PUT /v1/notifications/settings).
// Тонкий контроллер (AGENTS.md §5.5): парсинг тела и маппинг ошибок —
// максимум; вся логика и SQL — в SettingsService (user_id записи — только
// из сессии, тело PUT его не влияет — изоляция 02 §6.3). Оба маршрута под
// LoginFilter; без cookie — 401. path-id здесь нет — OwnershipGuard не
// применим (правило — для эндпоинтов с {id} в пути).
// Страница /profile — не обязательна (JSON самодостаточен); вёрстка —
// FE-дорожка, views/ не трогается.
class SettingsCtrl : public HttpController<SettingsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SettingsCtrl::get, "/v1/notifications/settings", Get,
                  "LoginFilter");
    ADD_METHOD_TO(SettingsCtrl::update, "/v1/notifications/settings", Put,
                  "LoginFilter");
    METHOD_LIST_END

    void get(const HttpRequestPtr& req,
             std::function<void(const HttpResponsePtr&)>&& cb);
    void update(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb);
};
