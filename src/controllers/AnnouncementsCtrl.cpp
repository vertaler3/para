#include "controllers/AnnouncementsCtrl.h"

#include <drogon/drogon.h>

#include "services/AnnouncementsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void AnnouncementsCtrl::feed(const HttpRequestPtr& req,
                             std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");

    // cursor — опциональный id последнего загруженного (новые сверху);
    // нечисловой → 400 (стиль days в ScheduleCtrl).
    std::optional<int64_t> cursor;
    const std::string cursorParam = req->getParameter("cursor");
    if (!cursorParam.empty())
    {
        try
        {
            cursor = std::stoll(cursorParam);
            if (*cursor <= 0)
                throw std::invalid_argument("cursor");
        }
        catch (const std::exception&)
        {
            cb(api::errorResponse("bad_request",
                                  "cursor — ожидается положительное число",
                                  k400BadRequest));
            return;
        }
    }

    AnnouncementsService::list(
        app().getDbClient("para"),
        session,
        cursor,
        [cb = std::move(cb)](const AnnouncementsService::Result& r) {
            if (r.ok)
                cb(HttpResponse::newHttpJsonResponse(r.body));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}
