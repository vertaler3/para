#include "services/MapService.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;
// ВАЖНО (урок RoomsService.cpp:19, повторён в LibraryService.cpp): внутри
// тела методов MapService имя `Result` резолвится в
// para::MapService::Result (class scope) — тип строк выборки в лямбдах
// пишется полностью: drogon::orm::Result.

namespace para {

namespace {

// Экранирование спецсимволов LIKE (% _ \) в q: пользовательский ввод —
// подстрочный поиск, а не шаблон (ESCAPE '\' в SQL; см. LibraryService).
std::string escapeLike(const std::string& q)
{
    std::string out;
    out.reserve(q.size());
    for (char c : q)
    {
        if (c == '\\' || c == '%' || c == '_')
            out += '\\';
        out += c;
    }
    return out;
}

}  // namespace

void MapService::buildings(drogon::orm::DbClientPtr db, ResultCb cb)
{
    *db << "SELECT id, name, address FROM buildings ORDER BY id ASC" >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
            {
                Json::Value b;
                b["id"] = r["id"].as<int64_t>();
                b["name"] = r["name"].as<std::string>();
                b["address"] = r["address"].as<std::string>();
                arr.append(std::move(b));
            }
            Json::Value body;
            body["buildings"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "MapService::buildings: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void MapService::rooms(drogon::orm::DbClientPtr db,
                       const RoomFilters& filters,
                       ResultCb cb)
{
    // Поиск кабинета (02 §5.2: q — номер, building_id): фиксированная SQL
    // «filters-or-empty», биндинги в порядке первых вхождений $N:
    //   $1 q-флаг ('' = без фильтра), $2 — LIKE-паттерн по name,
    //   $3/$4 — building_id (0 = без фильтра).
    const std::string qFlag = filters.q.empty() ? "" : "1";
    const std::string qLike = "%" + escapeLike(filters.q) + "%";
    *db << "SELECT r.id, r.name, r.building_id, b.name AS building,"
           " r.floor, r.kind FROM rooms r JOIN buildings b ON b.id ="
           " r.building_id"
           " WHERE ($1 = '' OR r.name LIKE $2 ESCAPE '\\')"
           "   AND ($3 = 0 OR r.building_id = $4)"
           " ORDER BY r.id ASC"
        << qFlag << qLike << filters.buildingId << filters.buildingId >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
            {
                Json::Value room;
                room["id"] = r["id"].as<int64_t>();
                room["name"] = r["name"].as<std::string>();
                room["building_id"] = r["building_id"].as<int64_t>();
                room["building"] = r["building"].as<std::string>();
                // floor/kind nullable по DDL 0006: не заполненные Э6а
                // строки (их в фикстурах нет) отдались бы null — честно.
                if (r["floor"].isNull())
                    room["floor"] = Json::Value();
                else
                    room["floor"] = r["floor"].as<int64_t>();
                if (r["kind"].isNull())
                    room["kind"] = Json::Value();
                else
                    room["kind"] = r["kind"].as<std::string>();
                arr.append(std::move(room));
            }
            Json::Value body;
            body["rooms"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "MapService::rooms: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

}  // namespace para
