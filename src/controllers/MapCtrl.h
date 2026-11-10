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
//
// Страничная плоскость (Э7, экран «Карта» 03 §4.9.6/5.16; Р-19 — путь
// вне /v1/ по постановке экрана):
//   GET /map               — SSR-страница views/map.csp: корпуса (чипы),
//       чипы этажей, кабинеты этажа и строка поиска кабинета. Без сессии
//       LoginFilter отвечает 303 → /login (страница). Данных страница не
//       содержит — корпуса и кабинеты грузит клиент из ТЕХ ЖЕ JSON-роутов
//       выше (GET /v1/map/buildings, GET /v1/map/rooms — 02 §5.2; паттерн
//       RoomsCtrl::roomsPage); query ?q= поисковой строки страница читает
//       на клиенте (location.search) и сразу выполняет поиск при загрузке;
//   GET /map/rooms         — алиас страницы: 303 → /map?q=<q> (q —
//       urlEncodeComponent исходного значения). Сюда ведут ссылки кабинета
//       из профилей людей (views/person.csp «Кабинет · …»; НЕ правятся —
//       алиас сохраняет их адреса живыми, постановка экрана карты).
//       С JSON-роутом /v1/map/rooms не конфликтует — пути различаются
//       префиксом /v1/.
class MapCtrl : public HttpController<MapCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MapCtrl::getBuildings, "/v1/map/buildings", Get,
                  "LoginFilter");
    ADD_METHOD_TO(MapCtrl::getRooms, "/v1/map/rooms", Get, "LoginFilter");
    ADD_METHOD_TO(MapCtrl::mapPage, "/map", Get, "LoginFilter");
    ADD_METHOD_TO(MapCtrl::mapRoomsAlias, "/map/rooms", Get, "LoginFilter");
    METHOD_LIST_END

    void getBuildings(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb);
    void getRooms(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void mapPage(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
    void mapRoomsAlias(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb);
};
