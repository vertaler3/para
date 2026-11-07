#include "controllers/RatingCtrl.h"

#include "services/PeopleService.h"
#include "services/RatingService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void RatingCtrl::show(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — факультет/роль запрашивающего сервис
    // берёт из БД по user_id сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");

    // query limit: опционален; нечисловой или вне 1..1000 → 400 (стиль
    // days в ScheduleCtrl: валидация аргумента — контроллер).
    int maxRows = 0;  // 0 — без ограничения
    const std::string rawLimit = req->getParameter("limit");
    if (!rawLimit.empty())
    {
        try
        {
            maxRows = std::stoi(rawLimit);
            if (maxRows < 1 || maxRows > 1000)
                throw std::out_of_range("limit");
        }
        catch (const std::exception&)
        {
            cb(api::errorResponse("bad_request",
                                  "limit — ожидается число от 1 до 1000",
                                  k400BadRequest));
            return;
        }
    }
    const std::string rawFaculty = req->getParameter("faculty");
    std::optional<std::string> faculty;
    if (!rawFaculty.empty())
        faculty = rawFaculty;

    const auto db = app().getDbClient("para");
    PeopleService::loadRequester(
        db, session.userId,
        [db, faculty = std::move(faculty), maxRows,
         cb = std::move(cb)](PeopleService::Status s,
                             PeopleService::Requester requester) mutable {
            if (s != PeopleService::Status::Ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            RatingService::rating(
                db, std::move(requester), faculty, maxRows,
                [cb = std::move(cb)](RatingService::Result r) mutable {
                    if (r.status == RatingService::Status::Ok)
                        cb(api::jsonResponse(std::move(r.body)));
                    else
                        cb(api::errorResponse(
                            "internal", "Внутренняя ошибка сервера",
                            k500InternalServerError));
                });
        });
}
