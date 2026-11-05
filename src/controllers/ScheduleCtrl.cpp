#include "controllers/ScheduleCtrl.h"

#include "services/ScheduleCache.h"
#include "services/ScheduleService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (02 §5.1: 500 —
// неожиданное, без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                ScheduleService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

}  // namespace

void ScheduleCtrl::getToday(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    ScheduleService::today(
        app().getDbClient("para"), session.userId, ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ScheduleCtrl::getWeek(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // offset — целое недель, ± (02 §5.2); отсутствует → 0; не число → 400.
    int offset = 0;
    const auto parsed = req->getOptionalParameter<int>("offset");
    if (parsed)
    {
        offset = *parsed;
    }
    else if (!req->getParameter("offset").empty())
    {
        cb(api::errorResponse("bad_request",
                              "offset — ожидается целое число (недель, ±)",
                              k400BadRequest));
        return;
    }
    ScheduleService::week(
        app().getDbClient("para"), session.userId, offset,
        &ScheduleCache::instance(), ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ScheduleCtrl::getChanges(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // days — целое 1..31 (02 §5.2: query days=7 по умолчанию).
    int days = 7;
    const auto parsed = req->getOptionalParameter<int>("days");
    if (parsed)
    {
        days = *parsed;
    }
    else if (!req->getParameter("days").empty())
    {
        cb(api::errorResponse("bad_request", "days — ожидается целое число",
                              k400BadRequest));
        return;
    }
    if (days < 1 || days > 31)
    {
        cb(api::errorResponse("bad_request", "days — ожидается целое 1..31",
                              k400BadRequest));
        return;
    }
    ScheduleService::changeList(
        app().getDbClient("para"), session.userId, days,
        ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}
