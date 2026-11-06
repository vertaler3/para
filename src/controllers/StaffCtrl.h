#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

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
// Дальше в StaffCtrl добавятся перенос расписания и объявления (Э6).
class StaffCtrl : public HttpController<StaffCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(StaffCtrl::queue, "/v1/staff/applications", Get,
                  "LoginFilter", "StaffFilter");
    ADD_METHOD_TO(StaffCtrl::changeStatus,
                  "/v1/staff/applications/{id}/status", Post, "LoginFilter",
                  "StaffFilter");
    METHOD_LIST_END

    void queue(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& cb);
    void changeStatus(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb,
                      int64_t applicationId);
};
