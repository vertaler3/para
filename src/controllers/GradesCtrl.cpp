#include "controllers/GradesCtrl.h"

#include "services/GradesService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (500 — неожиданное,
// без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                GradesService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

}  // namespace

void GradesCtrl::getGrades(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    GradesService::list(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradesService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void GradesCtrl::getSummary(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    GradesService::summary(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradesService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}
