#include "controllers/NewsCtrl.h"

#include "services/NewsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void NewsCtrl::feed(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — аудитория ленты только по профилю
    // этой сессии (02 §6.3); user_id сервис берёт сам из SessionInfo.
    const auto session = req->attributes()->get<SessionInfo>("session");

    // query: category / cursor. Нечисловой cursor → 400 (стиль cursor в
    // NotificationsFeedCtrl/AnnouncementsCtrl); словарь category и
    // существование строки-курсора — сервис (там же формат причин).
    NewsService::Query q;
    const std::string rawCategory = req->getParameter("category");
    if (!rawCategory.empty())
        q.category = rawCategory;
    const std::string rawCursor = req->getParameter("cursor");
    if (!rawCursor.empty())
    {
        try
        {
            q.cursorId = std::stoll(rawCursor);
            if (*q.cursorId <= 0)
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

    NewsService::feed(
        app().getDbClient("para"), session, q,
        [cb = std::move(cb)](NewsService::FeedResult r) mutable {
            switch (r.status)
            {
                case NewsService::FeedStatus::Ok:
                    cb(api::jsonResponse(std::move(r.body)));  // UTF-8 (гейт р1)
                    return;
                case NewsService::FeedStatus::BadRequest:
                    cb(api::errorResponse("bad_request", r.error,
                                          k400BadRequest));
                    return;
                case NewsService::FeedStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}
