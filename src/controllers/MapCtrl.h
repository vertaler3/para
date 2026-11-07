#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Карта корпусов (5.16; Э6б шаг 3; 02 §5.2). Тонкий контроллер
// (AGENTS.md §5.5): SQL и фильтры — MapService; роуты под LoginFilter —
// без cookie 401 от фильтра (02 §6.3). Каталоги публичные (корпуса и
// номера аудиторий — вымышленные), user-скоупа и OwnershipGuard нет.
//
//   GET /v1/map/buildings — корпуса → {"buildings": [{id, name,
//       address}, ...]};
//   GET /v1/map/rooms     — поиск кабинета: номер, корпус, этаж, тип
//       (rooms.building_id/floor/kind из 0006) → {"rooms": [...]};
//       query 02 §5.2: ?q=подстрока номера (гайд §9б: «?q=1207»),
//       ?building_id=N (не целое → 400, стиль branch_id в LibraryCtrl).
//
// Ошибки — единый формат 02 §5.1 (api::errorResponse); успешные —
// api::jsonResponse (кириллица сырыми UTF-8 байтами, гейт-конвенция Э6 р1).
class MapCtrl : public HttpController<MapCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MapCtrl::getBuildings, "/v1/map/buildings", Get,
                  "LoginFilter");
    ADD_METHOD_TO(MapCtrl::getRooms, "/v1/map/rooms", Get, "LoginFilter");
    METHOD_LIST_END

    void getBuildings(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb);
    void getRooms(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
};
