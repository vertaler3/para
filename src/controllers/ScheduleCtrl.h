#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Расписание группы (Э3 шаг 1; 02 §5.2 GET /v1/schedule/today,
// /v1/schedule/week?offset=, /v1/schedule/changes?days=). Тонкий контроллер
// (AGENTS.md §5.5): разбор query-параметров и маппинг ошибок — максимум;
// SQL и логика — ScheduleService, кэш недели — ScheduleCache. Все маршруты
// под LoginFilter: без cookie — 401 от фильтра (02 §6.3). Группа берётся
// из профиля сессии — path-id нет, OwnershipGuard не применим (правило —
// для эндпоинтов с {id} в пути).
class ScheduleCtrl : public HttpController<ScheduleCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ScheduleCtrl::getToday, "/v1/schedule/today", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ScheduleCtrl::getWeek, "/v1/schedule/week", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ScheduleCtrl::getChanges, "/v1/schedule/changes", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void getToday(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void getWeek(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
    void getChanges(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
};
