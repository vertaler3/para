#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Карта корпусов (5.16; Э6б шаг 3; 02 §5.2: GET /v1/map/buildings —
// «корпуса», GET /v1/map/rooms — «поиск кабинета: корпус, этаж, тип,
// query: q (номер), building_id»; данные — rooms.building_id/floor/kind
// из ALTER 0006, заполнены фикстурами Э6а, индекс idx_rooms_building
// (building_id, floor) — 02 §4.2). Весь SQL — параметризованный (AGENTS.md
// §5.5, 02 §6.4). Каталог — общий ресурс (OwnershipGuard не применяется —
// как аудитории/библиотека). Правила видимости 5.11–5.16 не добавляют
// ограничений: корпуса и номера аудиторий — публичный каталог вуза
// (вымышленный).
class MapService
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД → 500 контроллером (fail-closed)
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/map/buildings → {"buildings": [{id, name, address}, ...]} —
    // ORDER BY id. Пустая таблица — 200 с пустым массивом.
    static void buildings(drogon::orm::DbClientPtr db, ResultCb cb);

    // GET /v1/map/rooms → {"rooms": [{id, name, building_id, building,
    // floor, kind}, ...]} — поиск кабинета по номеру (гайд §9б:
    // «?q=1207 → корпус, этаж, тип»).
    // Query-фильтры 02 §5.2 — «filters-or-empty» (одна фиксированная
    // параметризованная SQL, паттерн kQueueSql ApplicationsService;
    // порядок первых вхождений $N в тексте = порядку биндинга — правило
    // Э5 шаг 2, повторных вхождений нет):
    //   q         — подстрока по номеру кабинета (name LIKE '%q%',
    //               ESCAPE '\' — спецсимволы % _ \ экранирует сервис;
    //               «204» находит «Б-204» — частный случай подстроки);
    //               пустая строка = без фильтра;
    //   buildingId— 0 = без фильтра, иначе rooms.building_id = $N.
    // INNER JOIN buildings: строки rooms без корпуса (building_id NULL —
    // не заполнены Э6а) карту не показывают — честнее пустоты в полях.
    // Сортировка по id — стиль каталогов (02 §5.2 порядок не оговаривает).
    struct RoomFilters
    {
        std::string q;         // query q, пустая = без фильтра
        int64_t buildingId = 0;  // query building_id, 0 = без фильтра
    };
    static void rooms(drogon::orm::DbClientPtr db,
                      const RoomFilters& filters,
                      ResultCb cb);
};

}  // namespace para
