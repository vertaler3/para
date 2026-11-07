#include "controllers/GradebookCtrl.h"

#include "services/GradebookService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void GradebookCtrl::show(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");
    GradebookService::gradebook(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradebookService::Result r) mutable {
            // Ok — 200 UTF-8 (emitUTF8, Э6 правки р1); Error — 500
            // fail-closed, не пустая книжка.
            if (r.status == GradebookService::Status::Ok)
                cb(api::jsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}
