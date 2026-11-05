#include "controllers/ApplicationsCtrl.h"

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат Result-сервисов → ответ: ok — JSON (опционально с кодом 201),
// иначе 500 без деталей (02 §5.1).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                ApplicationsService::Result r,
                HttpStatusCode successCode = k200OK)
{
    if (!r.ok)
    {
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
        return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(std::move(r.body));
    resp->setStatusCode(successCode);
    cb(std::move(resp));
}

// Вердикт guard'а → ответ (02 §6.3/§5.1) — как в GradesCtrl: чужое — 403,
// несуществующее — 404, ошибка БД — 500 (fail-closed). Own → false, путь
// к данным продолжает вызывающий.
bool sendVerdict(std::function<void(const HttpResponsePtr&)>& cb,
                 OwnershipGuard::Verdict v)
{
    switch (v)
    {
        case OwnershipGuard::Verdict::Own:
            return false;
        case OwnershipGuard::Verdict::NotFound:
            cb(api::errorResponse("not_found", "Заявка не найдена",
                                  k404NotFound));
            return true;
        case OwnershipGuard::Verdict::Forbidden:
            // Формулировка — пример из 02 §5.1.
            cb(api::errorResponse("forbidden", "Доступ только к своим данным",
                                  k403Forbidden));
            return true;
        case OwnershipGuard::Verdict::Error:
            cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                  k500InternalServerError));
            return true;
    }
    return true;  // недостижимо при полном switch над enum
}

}  // namespace

void ApplicationsCtrl::getTypes(const HttpRequestPtr&,
                                std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Каталог — constexpr-константа src/models (02 §4.3); БД не нужна.
    cb(HttpResponse::newHttpJsonResponse(ApplicationsService::catalog()));
}

void ApplicationsCtrl::create(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                              k400BadRequest));
        return;
    }
    const auto spec = ApplicationsService::parseCreate(*json);
    if (!spec.ok)
    {
        cb(api::errorResponse("bad_request", spec.errorMessage,
                              k400BadRequest));
        return;
    }
    ApplicationsService::create(
        app().getDbClient("para"), session.userId, spec,
        [cb = std::move(cb)](ApplicationsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r), k201Created);
        });
}

void ApplicationsCtrl::list(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // query status — опциональный фильтр; значение сверяется со словарём
    // CHECK DDL (02 §4.1) — неизвестное → 400 (стиль days в ScheduleCtrl).
    std::optional<std::string> statusFilter;
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
        statusFilter = status;
    }
    ApplicationsService::list(
        app().getDbClient("para"), session.userId, statusFilter,
        [cb = std::move(cb)](ApplicationsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ApplicationsCtrl::getOne(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb,
                              int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Правило код-ревью (OwnershipGuard.h): path-id → guard ДО любой отдачи
    // данных (02 §6.3 п.3). Выборка после guard'а — по PK без user_id
    // (владение проверяется ровно один раз, здесь; паттерн getGrade Э4).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            ApplicationsService::one(
                db, applicationId,
                [cb = std::move(cb)](ApplicationsService::OneResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::One::Found:
                            cb(HttpResponse::newHttpJsonResponse(
                                std::move(r.body)));
                            return;
                        case ApplicationsService::One::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::One::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void ApplicationsCtrl::updateDraft(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Guard ДО разбора тела и ДО проверки статуса (постановка шага 2):
    // чужая не-draft заявка → 403, а не 409/400. req захватывается (сессия
    // проверена фильтром, тело читается после Own).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, req,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            const auto json = req->getJsonObject();
            if (json == nullptr)
            {
                cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                                      k400BadRequest));
                return;
            }
            const auto spec = ApplicationsService::parsePayloadJson(*json);
            if (!spec.ok)
            {
                cb(api::errorResponse("bad_request", spec.errorMessage,
                                      k400BadRequest));
                return;
            }
            ApplicationsService::updateDraft(
                db, applicationId, spec.text,
                [cb = std::move(cb)](
                    ApplicationsService::UpdateResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::Update::Ok:
                            cb(HttpResponse::newHttpJsonResponse(
                                std::move(r.body)));
                            return;
                        case ApplicationsService::Update::NotDraft:
                            // 02 §5.2: «409 (статус ≠ draft)».
                            cb(api::errorResponse(
                                "invalid_status_transition",
                                "Править можно только черновик заявки",
                                k409Conflict));
                            return;
                        case ApplicationsService::Update::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::Update::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void ApplicationsCtrl::sendMessage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Guard ДО разбора тела и ДО проверки статуса (постановка шага 2).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, req, userId = session.userId,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            const auto json = req->getJsonObject();
            if (json == nullptr)
            {
                cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                                      k400BadRequest));
                return;
            }
            const auto spec = ApplicationsService::parseMessageText(*json);
            if (!spec.ok)
            {
                cb(api::errorResponse("bad_request", spec.errorMessage,
                                      k400BadRequest));
                return;
            }
            ApplicationsService::sendMessage(
                db, applicationId, spec.text, userId,
                [cb = std::move(cb)](
                    ApplicationsService::MessageResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::Message::Ok:
                        {
                            // 201 — событие диалога записано (02 §5.2);
                            // при need_info это событие перехода
                            // need_info→in_progress (С7.6).
                            auto resp = HttpResponse::newHttpJsonResponse(
                                std::move(r.body));
                            resp->setStatusCode(k201Created);
                            cb(std::move(resp));
                            return;
                        }
                        case ApplicationsService::Message::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::Message::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}
