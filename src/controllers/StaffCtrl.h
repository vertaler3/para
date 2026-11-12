#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

namespace para {
class ScheduleCache;  // инвалидация кэша недели группы (хук Э3) — Э6 шаг 4
}

// Рабочий стол сотрудника (Э5 шаг 3; 02 §5.2 staff-строки; С8). Тонкий
// контроллер: парсинг query/body, маппинг исходов сервиса в HTTP; SQL и
// автомат — ApplicationsService/AppWorkflow. Оба маршрута под LoginFilter +
// StaffFilter (02 §6.3 RBAC: студент на /v1/staff/* → 403; фильтр в
// METHOD_LIST идёт после LoginFilter — роль приходит из сессии).
//
//   GET  /v1/staff/applications           — очередь заявок: query status /
//       type / overdue=1 (02 §5.2), сортировка по updated_at, в строках —
//       владелец (fio/группа) и признак «горит» (С8.2–С8.3);
//   POST /v1/staff/applications/{id}/status — смена статуса по автомату
//       (недопустимый переход → 409; несуществующий id → 404 — OwnershipGuard
//       НЕ вызывается сознательно: staff видит все заявки, постановка шага 3;
//       guard — для student-owned объектов) + автоуведомление автору заявки
//       (notifications, в одной транзакции с UPDATE и событием).
//
// Ответы JSON; htmx-фрагмент карточки по HX-Request (02 §3.2 поток 2) —
// при интеграции FE-дорожки (views/ — не BE; Э5 FE не начиналась).
//
// Э6 шаг 4 (02 §5.2 staff-строки):
//   POST /v1/staff/schedule/changes — перенос/отмена/замена занятия
//       (prev_* «было → стало»), ПОСЛЕ коммита — инвалидация кэша недели
//       группы (хук Э3) и авторассылка type='schedule' студентам группы
//       через NotifyDispatcher (201 · 404 · 400);
//   GET  /v1/staff/dashboard — счётчики очереди заявок + свежие изменения
//       расписания (200);
//   POST /v1/staff/announcements — публикация объявления; БЕЗ push и БЕЗ
//       уведомлений — только лента (01 §4.5) (201). 5.17: необязательное
//       поле kind ('announcement' — по умолчанию | 'news') — kind='news'
//       публикует НОВОСТЬ тем же роутом (таблица news 0007: category /
//       event_at / location, published_at = сейчас, аудитория — 'all' или
//       ровно одна цель; БЕЗ push/уведомлений — как объявления, 01 §4.5);
//       тело без kind / с kind='announcement' — прежнее поведение
//       байт-в-байт (совместимость Э6); CSRF — прежний CsrfCheck из
//       LoginFilter, новых exempt нет.
//
// Страничная плоскость (Э7, экран «Стол сотрудника» — последний экран
// Э7; 03 §4.8; путь вне /v1/ — по постановке экрана, как /schedule —
// Р-19 в docs/decisions.md):
//   GET /staff          — views/staff.csp: сводка, очередь, перенос,
//                         объявления. Роут под LoginFilter БЕЗ StaffFilter:
//                         СТРАНИЦА не отвечает API-JSON — роль проверяет
//                         КОНТРОЛЛЕР сам по атрибуту сессии "session"
//                         (его туда кладёт LoginFilter после
//                         SessionService::findValid — паттерн
//                         ScheduleCtrl/DashboardCtrl): не staff → HTTP 403
//                         + HTML-карточка «Доступ только для сотрудников»
//                         (views/staff.csp mode="forbidden" — тот же вид
//                         карточки, что 403 у заявок; 03 §5). ЭТО НЕ МЕНЯЕТ
//                         API: StaffFilter остаётся на всех /v1/staff/*
//                         роутах выше и отвечает на них прежним JSON 403
//                         {error:{code,message}} (02 §5.1/§5.2) — страница
//                         и API-плоскость разделены путём (Р-19);
//   GET /staff/app-card — htmx-ФРАГМЕНТ карточки очереди views/staff/
//                         app_card.csp (паттерн /schedule/segment, Р-19):
//                         при HX-Request: true — партиал одной карточки по
//                         query id (целое; иначе 400 HTML-фрагментом —
//                         02 §5.1 «ошибки валидации — HTML-фрагментом для
//                         htmx»); прямой заход без HX-Request → 303 на
//                         /staff. Роль — та же проверка контроллера:
//                         не staff → 403 HTML-фрагментом (notice). Данные
//                         карточки — ApplicationsService::queue (ТОТ ЖЕ
//                         метод, что за JSON GET /v1/staff/applications;
//                         выборка нужной строки — по id в памяти
//                         контроллера: новой SQL нет, SQL в контроллерах
//                         запрещён — AGENTS §5.5, src/services/ — BE-
//                         дорожка). Потребитель фрагмента — app.js: после
//                         успешного POST /v1/staff/applications/{id}/status
//                         заменяет карточку очереди этим партиалом
//                         (htmx.ajax, outerHTML) — «htmx-перерисовка
//                         карточки», 03 §4.8; 409 недопустимого перехода
//                         карточку НЕ перерисовывает — inline-предупреждение
//                         в слоте data-app-warn.
class StaffCtrl : public HttpController<StaffCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(StaffCtrl::queue, "/v1/staff/applications", Get,
                  "LoginFilter", "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::changeStatus,
                  "/v1/staff/applications/{id}/status", Post, "LoginFilter",
                  "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::scheduleChange, "/v1/staff/schedule/changes",
                  Post, "LoginFilter", "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::dashboard, "/v1/staff/dashboard", Get,
                  "LoginFilter", "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::publishAnnouncement, "/v1/staff/announcements",
                  Post, "LoginFilter", "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::staffPage, "/staff", Get, "LoginFilter");
    ADD_METHOD_TO(StaffCtrl::appCardFragment, "/staff/app-card", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void queue(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& cb);
    void changeStatus(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb,
                      int64_t applicationId);
    void scheduleChange(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& cb);
    void dashboard(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void publishAnnouncement(const HttpRequestPtr& req,
                             std::function<void(const HttpResponsePtr&)>&& cb);
    void staffPage(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void appCardFragment(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb);
};
