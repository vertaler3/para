#include "controllers/MapCtrl.h"

#include "services/MapService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void MapCtrl::getBuildings(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — каталог общий (02 §6.3; шапка MapService).
    MapService::buildings(
        app().getDbClient("para"),
        [cb = std::move(cb)](MapService::Result r) mutable {
            if (r.ok)
                cb(api::jsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}

void MapCtrl::getRooms(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // query контракта 02 §5.2: q (подстрока номера кабинета), building_id
    // («не целое → 400» — стиль branch_id в LibraryCtrl / capacity в
    // RoomsCtrl).
    MapService::RoomFilters filters;
    filters.q = req->getParameter("q");
    const auto building = req->getOptionalParameter<int64_t>("building_id");
    if (building)
    {
        filters.buildingId = *building;
    }
    else if (!req->getParameter("building_id").empty())
    {
        cb(api::errorResponse("bad_request",
                              "building_id — ожидается целое число",
                              k400BadRequest));
        return;
    }
    MapService::rooms(
        app().getDbClient("para"), filters,
        [cb = std::move(cb)](MapService::Result r) mutable {
            if (r.ok)
                cb(api::jsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}
