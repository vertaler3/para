#include "controllers/RoomsCtrl.h"

#include "services/RoomsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (500 — неожиданное,
// без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                RoomsService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

}  // namespace

void RoomsCtrl::getRooms(const HttpRequestPtr&,  // сессия каталогу не нужна:
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — доступ только по cookie (02 §6.3).
    RoomsService::catalog(
        app().getDbClient("para"),
        [cb = std::move(cb)](RoomsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void RoomsCtrl::getSlots(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb,
                         int64_t roomId)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3):
    // «ваша бронь» в сетке — по нему (RoomsService::slots).
    const auto session = req->attributes()->get<para::SessionInfo>("session");

    // ?date= — YYYY-MM-DD (02 §5.2 «query: date»); отсутствует → сегодня
    // UTC (как /v1/schedule/today); невалидный/некалендарный → 400
    // bad_request (02 §5.1), сетку по такой дате не строим.
    const std::string rawDate = req->getParameter("date");
    const std::string date =
        rawDate.empty() ? RoomsService::todayUtc() : rawDate;
    if (!RoomsService::validDate(date))
    {
        cb(api::errorResponse("bad_request",
                              "date — ожидается дата YYYY-MM-DD",
                              k400BadRequest));
        return;
    }

    RoomsService::slots(
        app().getDbClient("para"), roomId, session.userId, date,
        [cb = std::move(cb)](RoomsService::SlotsResult r) mutable {
            switch (r.status)
            {
                case RoomsService::SlotsStatus::Ok:
                    cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
                    return;
                case RoomsService::SlotsStatus::NotFound:
                    cb(api::errorResponse("not_found", "Аудитория не найдена",
                                          k404NotFound));
                    return;
                case RoomsService::SlotsStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}
