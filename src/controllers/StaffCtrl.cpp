#include "controllers/StaffCtrl.h"

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Вердикт смены статуса → ответ (02 §5.2: 200 · 403 · 404 · 409; 403 отдаёт
// StaffFilter до контроллера).
void sendStatusChange(std::function<void(const HttpResponsePtr&)> cb,
                      ApplicationsService::StatusChangeResult r)
{
    switch (r.status)
    {
        case ApplicationsService::StatusChange::Ok:
            cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
            return;
        case ApplicationsService::StatusChange::InvalidTransition:
            // Автомат шага 1: переход вне матрицы (02 §5.1: 409 —
            // недопустимый переход статуса заявки; 02 §5.2 поток 2:
            // «issued → in_progress нет → 409»).
            cb(api::errorResponse("invalid_status_transition",
                                  "Недопустимый переход статуса заявки",
                                  k409Conflict));
            return;
        case ApplicationsService::StatusChange::NotFound:
            cb(api::errorResponse("not_found", "Заявка не найдена",
                                  k404NotFound));
            return;
        case ApplicationsService::StatusChange::Error:
            cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                  k500InternalServerError));
            return;
    }
}

}  // namespace

void StaffCtrl::queue(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Фильтры query (02 §5.2: status, type, overdue=1) — валидность значений
    // здесь, до сервиса (неизвестные → 400; стиль days в ScheduleCtrl).
    ApplicationsService::QueueFilters filters;
    const std::string status = req->getParameter("status");
    if (!status.empty())
    {
        bool known = false;
        for (const auto& s : models::kApplicationStatuses)
            if (s == status)
            {
                known = true;
                break;
            }
        if (!known)
        {
            cb(api::errorResponse("bad_request",
                                  "status — недопустимое значение",
                                  k400BadRequest));
            return;
        }
        filters.status = status;
    }
    const std::string type = req->getParameter("type");
    if (!type.empty())
    {
        if (!models::isKnownApplicationType(type))
        {
            cb(api::errorResponse("bad_request",
                                  "type — неизвестный ключ каталога типов",
                                  k400BadRequest));
            return;
        }
        filters.type = type;
    }
    const std::string overdue = req->getParameter("overdue");
    if (!overdue.empty() && overdue != "1")
    {
        cb(api::errorResponse("bad_request", "overdue — ожидается 1",
                              k400BadRequest));
        return;
    }
    filters.overdueOnly = overdue == "1";

    ApplicationsService::queue(
        app().getDbClient("para"), filters,
        [cb = std::move(cb)](ApplicationsService::Result r) mutable {
            if (!r.ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
        });
}

void StaffCtrl::changeStatus(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    // OwnershipGuard сознательно НЕ вызывается (постановка шага 3: staff
    // видит все заявки — guard для student-owned объектов; правило
    // код-ревью — про эндпоинты с {id} студента). Несуществующий id → 404
    // из сервиса (NotFound автомата).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                              k400BadRequest));
        return;
    }
    if (!json->isMember("to_status") || !(*json)["to_status"].isString())
    {
        cb(api::errorResponse("bad_request",
                              "to_status — ожидается статус заявки",
                              k400BadRequest));
        return;
    }
    const std::string toStatus = (*json)["to_status"].asString();
    bool known = false;
    for (const auto& s : models::kApplicationStatuses)
        if (s == toStatus)
        {
            known = true;
            break;
        }
    if (!known)
    {
        // Неизвестный статус — ошибка тела (400), а не перехода (409).
        cb(api::errorResponse("bad_request",
                              "to_status — недопустимое значение",
                              k400BadRequest));
        return;
    }
    std::optional<std::string> comment;
    if (json->isMember("comment"))
    {
        if (!(*json)["comment"].isString())
        {
            cb(api::errorResponse("bad_request",
                                  "comment — ожидается строка",
                                  k400BadRequest));
            return;
        }
        comment = (*json)["comment"].asString();
    }

    ApplicationsService::staffChangeStatus(
        app().getDbClient("para"), applicationId, toStatus, comment,
        session.userId,
        [cb = std::move(cb)](
            ApplicationsService::StatusChangeResult r) mutable {
            sendStatusChange(std::move(cb), std::move(r));
        });
}
