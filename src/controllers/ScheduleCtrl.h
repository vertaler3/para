#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Расписание группы (Э3 шаг 1; 02 §5.2 GET /v1/schedule/today,
// /v1/schedule/week?offset=, /v1/schedule/changes?days=). Тонкий контроллер
// (AGENTS.md §5.5): разбор query-параметров и маппинг ошибок — максимум;
// SQL и логика — ScheduleService, кэш недели — ScheduleCache. Группа берётся
// из профиля сессии — path-id нет, OwnershipGuard не применим (правило —
// для эндпоинтов с {id} в пути).
//
// Плоскости (02 §3.2, §5.2, §6.3; путь фрагмента вне /v1/ — по постановке
// экрана, отклонение от 02 §5.1 зафиксировано Р-19 в docs/decisions.md):
//   /v1/schedule/*         — JSON API: без cookie LoginFilter отвечает 401
//                            (02 §6.3);
//   GET /schedule          — SSR-страница views/schedule.csp (03 §4.3,
//                            Э7); query: segment=today|week (по умолчанию
//                            today), offset — сегмент «Неделя», date —
//                            день deep_link (/schedule?date=… — канон
//                            deep_link уведомлений Э6 и тапа по переносу,
//                            Р-17: «Сегодня» показывает этот день, «Неделя»
//                            — неделю, содержащую дату; date приоритетнее
//                            offset);
//   GET /schedule/segment  — htmx-фрагмент сегмента (Э7, Р-18): при
//                            HX-Request: true — только партиал
//                            schedule_today / schedule_week (outerHTML-
//                            обмен #schedule-segment); прямой заход без
//                            HX-Request → 303 на /schedule. segment —
//                            today|week, offset — целое [-4..4], date —
//                            календарная дата YYYY-MM-DD, иначе
//                            400 bad_request (02 §5.1; для htmx-запроса —
//                            HTML-фрагмент, Р-19). Без cookie фильтр
//                            отвечает 303 → /login (страница,
//                            LoginFilter.cpp isApi=false, 02 §6.3).
// Данные страницы/фрагмента — те же методы ScheduleService, что стоят за
// JSON API (day/week/changeList); в шаблон уходят сериализованные тела —
// экранирование при выводе делает партиал (02 §6.4, Р-16).
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
    ADD_METHOD_TO(ScheduleCtrl::page, "/schedule", Get, "LoginFilter");
    ADD_METHOD_TO(ScheduleCtrl::segment, "/schedule/segment", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void getToday(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void getWeek(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
    void getChanges(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
    void page(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void segment(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
};
