#include "controllers/MapCtrl.h"

#include <drogon/drogon.h>
#include <drogon/utils/Utilities.h>

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

// --- Страничная плоскость (Э7: GET /map, GET /map/rooms; см. шапку) -------

void MapCtrl::mapPage(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 /login —
    // правка фильтра Э7/Р-19, как RoomsCtrl::roomsPage). Тонкая страница
    // без данных (AGENTS §5.5): корпуса/кабинеты грузит клиент из тех же
    // JSON-роутов этого контроллера (GET /v1/map/buildings, GET
    // /v1/map/rooms — 02 §5.2); query ?q= страница читает на клиенте
    // (location.search) и сразу выполняет поиск при загрузке. Состояния
    // 03 §5 — в шаблоне и static/js/app.js.
    (void)req;
    HttpViewData data;
    data.insert("pageTitle", std::string("Карта корпусов · Пара"));
    // Экран НЕ таб (03 §3.1 — вход из профиля/профиля человека,
    // 03 §4.9.6/5.16): пустой activeTab — ни один пункт таб-бара не
    // получает aria-current.
    data.insert("activeTab", std::string(""));
    cb(HttpResponse::newHttpViewResponse("map", std::move(data)));
}

void MapCtrl::mapRoomsAlias(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Алиас ссылок кабинета из профилей людей (views/person.csp «Кабинет
    // · …» ведёт на /map/rooms?q=<номер> — править их НЕ нужно):
    // 303 → /map?q=<q>, q — то же значение, url-экранированное
    // (urlEncodeComponent — экранирует и зарезервированные символы
    // запроса); пустой/отсутствующий q — на страницу без query.
    // Сессию проверил LoginFilter (как у страницы).
    const std::string q = req->getParameter("q");
    const std::string location =
        q.empty() ? "/map" : "/map?q=" + drogon::utils::urlEncodeComponent(q);
    cb(HttpResponse::newRedirectionResponse(location, k303SeeOther));
}
