#include "controllers/StaffCtrl.h"

#include <drogon/drogon.h>

#include "models/Applications.h"
#include "services/AnnouncementsService.h"
#include "services/ApplicationsService.h"
#include "services/ScheduleCache.h"
#include "services/ScheduleChangeService.h"
#include "services/StaffDashboard.h"
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

// ==================== Э6 шаг 4: расписание, дашборд, объявления ============

void StaffCtrl::scheduleChange(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                              k400BadRequest));
        return;
    }

    // Словарь action (02 §5.2): move/cancel/substitute — вне словаря → 400.
    if (!json->isMember("action") || !(*json)["action"].isString())
    {
        cb(api::errorResponse("bad_request",
                              "action — ожидается move, cancel или substitute",
                              k400BadRequest));
        return;
    }
    const auto action =
        ScheduleChangeService::actionFromString((*json)["action"].asString());
    if (!action.has_value())
    {
        cb(api::errorResponse("bad_request",
                              "action — ожидается move, cancel или substitute",
                              k400BadRequest));
        return;
    }

    ScheduleChangeService::Input in;
    in.action = *action;
    if (!json->isMember("event_id") || !(*json)["event_id"].isIntegral())
    {
        cb(api::errorResponse("bad_request",
                              "event_id — ожидается целое число",
                              k400BadRequest));
        return;
    }
    in.eventId = (*json)["event_id"].asInt64();
    if (in.eventId <= 0)
    {
        cb(api::errorResponse("bad_request",
                              "event_id — ожидается положительное число",
                              k400BadRequest));
        return;
    }
    if (json->isMember("starts_at"))
    {
        if (!(*json)["starts_at"].isString())
        {
            cb(api::errorResponse("bad_request",
                                  "starts_at — ожидается строка ISO «…Z»",
                                  k400BadRequest));
            return;
        }
        in.startsAt = (*json)["starts_at"].asString();
    }
    if (json->isMember("room"))
    {
        if (!(*json)["room"].isString())
        {
            cb(api::errorResponse("bad_request",
                                  "room — ожидается строка", k400BadRequest));
            return;
        }
        in.room = (*json)["room"].asString();
    }
    if (json->isMember("teacher_id"))
    {
        if (!(*json)["teacher_id"].isIntegral())
        {
            cb(api::errorResponse("bad_request",
                                  "teacher_id — ожидается целое число",
                                  k400BadRequest));
            return;
        }
        in.teacherId = (*json)["teacher_id"].asInt64();
    }
    if (json->isMember("reason"))
    {
        if (!(*json)["reason"].isString())
        {
            cb(api::errorResponse("bad_request", "reason — ожидается строка",
                                  k400BadRequest));
            return;
        }
        in.reason = (*json)["reason"].asString();
    }

    // Хук Э3: инвалидация кэша недели группы — ПОСЛЕ коммита изменения
    // (внутри сервиса), до авторассылки.
    auto invalidate = [](int64_t groupId) {
        ScheduleCache::instance().invalidate(groupId);
    };

    ScheduleChangeService::applyChange(
        app().getDbClient("para"), session, in, std::move(invalidate),
        [cb = std::move(cb)](const ScheduleChangeService::Result& r) {
            switch (r.outcome)
            {
                case ScheduleChangeService::Outcome::Ok:
                {
                    // 201 Created — внесено изменение + рассылка запущена
                    // (02 §5.2); тело — читаемый UTF-8 (правки по гейту
                    // раунд 1: сырой поиск «История России» в event).
                    cb(api::jsonResponse(r.body, k201Created));
                    return;
                }
                case ScheduleChangeService::Outcome::NotFound:
                    cb(api::errorResponse("not_found", "Занятие не найдено",
                                          k404NotFound));
                    return;
                case ScheduleChangeService::Outcome::BadRequest:
                    cb(api::errorResponse(
                        "bad_request",
                        "Недопустимое действие или поля переноса",
                        k400BadRequest));
                    return;
                case ScheduleChangeService::Outcome::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

void StaffCtrl::dashboard(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    (void)req;  // сессия не нужна: staff-роут, счётчики общие
    StaffDashboard::build(
        app().getDbClient("para"),
        [cb = std::move(cb)](const StaffDashboard::Result& r) {
            if (r.ok)
                cb(api::jsonResponse(r.body));  // читаемый UTF-8 (гейт раунд 1)
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}

void StaffCtrl::publishAnnouncement(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                              k400BadRequest));
        return;
    }
    if (!json->isMember("title") || !(*json)["title"].isString() ||
        (*json)["title"].asString().empty())
    {
        cb(api::errorResponse("bad_request",
                              "title — ожидается непустая строка",
                              k400BadRequest));
        return;
    }
    if (!json->isMember("body") || !(*json)["body"].isString() ||
        (*json)["body"].asString().empty())
    {
        cb(api::errorResponse("bad_request",
                              "body — ожидается непустая строка",
                              k400BadRequest));
        return;
    }

    // audience: отсутствует → 'all'; строка "all"/JSON-текст массива;
    // массив JSON (дружелюбная форма тела) → сериализуем в текст колонки.
    AnnouncementsService::Audience audience;
    if (!json->isMember("audience"))
    {
        audience.all = true;  // DEFAULT DDL 0003
    }
    else if ((*json)["audience"].isString())
    {
        if (!AnnouncementsService::parseAudience(
                (*json)["audience"].asString(), audience))
        {
            cb(api::errorResponse(
                "bad_request",
                "audience — ожидается \"all\" или массив [\"group:<id>\", "
                "\"course:<id>\"]",
                k400BadRequest));
            return;
        }
    }
    else if ((*json)["audience"].isArray())
    {
        Json::StreamWriterBuilder b;
        b["indentation"] = "";
        const std::string text = Json::writeString(b, (*json)["audience"]);
        if (!AnnouncementsService::parseAudience(text, audience))
        {
            cb(api::errorResponse(
                "bad_request",
                "audience — ожидается \"all\" или массив [\"group:<id>\", "
                "\"course:<id>\"]",
                k400BadRequest));
            return;
        }
    }
    else
    {
        cb(api::errorResponse(
            "bad_request",
            "audience — ожидается \"all\" или массив [\"group:<id>\", "
            "\"course:<id>\"]",
            k400BadRequest));
        return;
    }

    // Без push и без уведомлений — только лента (01 §4.5, гайд §9).
    AnnouncementsService::publish(
        app().getDbClient("para"), session.userId,
        (*json)["title"].asString(), (*json)["body"].asString(), audience,
        [cb = std::move(cb)](const AnnouncementsService::Result& r) {
            if (r.ok)
            {
                cb(api::jsonResponse(r.body, k201Created));
            }
            else
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
            }
        });
}
