#include "services/CanteensService.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;
// ВАЖНО (урок RoomsService.cpp:19, повторён в LibraryService.cpp): внутри
// тела методов CanteensService имя `Result` резолвится в
// para::CanteensService::Result (class scope) — тип строк выборки в лямбдах
// пишется полностью: drogon::orm::Result.

namespace para {

namespace {

Json::Value menuJson(const Row& r)
{
    Json::Value it;
    it["id"] = r["id"].as<int64_t>();
    it["name"] = r["name"].as<std::string>();
    it["category"] = r["category"].as<std::string>();
    it["is_combo"] = r["is_combo"].as<int64_t>() != 0;  // «комплексные»
    it["price"] = r["price"].as<int64_t>();             // ₽
    // КБЖУ nullable: вода без КБЖУ (fixtures Э6б) → null, не 0.
    if (r["kcal"].isNull())
        it["kcal"] = Json::Value();
    else
        it["kcal"] = r["kcal"].as<int64_t>();
    for (const char* f : {"protein", "fat", "carbs"})
    {
        if (r[f].isNull())
            it[f] = Json::Value();
        else
            it[f] = r[f].as<double>();
    }
    return it;
}

}  // namespace

void CanteensService::list(drogon::orm::DbClientPtr db, ResultCb cb)
{
    // Список столовых: корпус и часы (02 §5.2), ORDER BY id — стиль
    // каталога RoomsService::catalog / LibraryService::branches.
    *db << "SELECT c.id, c.name, c.building_id, b.name AS building,"
           " c.hours FROM canteens c JOIN buildings b ON b.id ="
           " c.building_id ORDER BY c.id ASC" >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
            {
                Json::Value c;
                c["id"] = r["id"].as<int64_t>();
                c["name"] = r["name"].as<std::string>();
                c["building_id"] = r["building_id"].as<int64_t>();
                c["building"] = r["building"].as<std::string>();
                c["hours"] = r["hours"].as<std::string>();
                arr.append(std::move(c));
            }
            Json::Value body;
            body["canteens"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "CanteensService::list: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void CanteensService::canteenById(
    drogon::orm::DbClientPtr db,
    int64_t canteenId,
    std::function<void(Lookup, Json::Value)> cb)
{
    *db << "SELECT c.id, c.name, c.building_id, b.name AS building, c.hours"
           " FROM canteens c JOIN buildings b ON b.id = c.building_id"
           " WHERE c.id = $1"
        << canteenId >>
        [cb](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                cb(Lookup::NotFound, Json::Value());
                return;
            }
            const auto& r = rows[0];
            Json::Value c;
            c["id"] = r["id"].as<int64_t>();
            c["name"] = r["name"].as<std::string>();
            c["building_id"] = r["building_id"].as<int64_t>();
            c["building"] = r["building"].as<std::string>();
            c["hours"] = r["hours"].as<std::string>();
            cb(Lookup::Found, std::move(c));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "CanteensService::canteenById: " << e.base().what();
            cb(Lookup::Error, Json::Value());  // fail-closed
        };
}

void CanteensService::menu(drogon::orm::DbClientPtr db,
                           int64_t canteenId,
                           std::function<void(MenuResult)> cb)
{
    // Двухфазно, как RoomsService::slots (roomById → сетка): существование
    // столовой (404, 02 §5.2) → меню по idx_menu_items_canteen. Линейная
    // цепочка (урок Э5а шаг 3 — без самозахватывающихся рекурсивных
    // shared_ptr-функций); cb копией в обеих ветках, стреляет ровно одна.
    canteenById(
        db, canteenId,
        [db, canteenId, cb](Lookup lookup, Json::Value canteen) mutable {
            switch (lookup)
            {
                case Lookup::NotFound:
                {
                    MenuResult r;
                    r.status = MenuStatus::NotFound;
                    cb(std::move(r));
                    return;
                }
                case Lookup::Error:
                {
                    MenuResult r;
                    r.status = MenuStatus::Error;
                    cb(std::move(r));
                    return;
                }
                case Lookup::Found:
                    break;
            }
            *db << "SELECT id, name, category, is_combo, price, kcal,"
                   " protein, fat, carbs FROM menu_items WHERE canteen_id"
                   " = $1 ORDER BY id ASC"
                << canteenId >>
                [cb, canteen = std::move(canteen)](
                    const drogon::orm::Result& rows) mutable {
                    Json::Value arr(Json::arrayValue);
                    for (const auto& r : rows)
                        arr.append(menuJson(r));
                    // Шапка столовой (контракт шапки CanteensCtrl.h) —
                    // чтобы FE не делал второй запрос: имя столовой в
                    // поле "canteen", а не "name".
                    Json::Value body;
                    body["canteen_id"] = canteen["id"];
                    body["canteen"] = canteen["name"];
                    body["building"] = canteen["building"];
                    body["hours"] = canteen["hours"];
                    body["menu"] = std::move(arr);
                    MenuResult r;
                    r.status = MenuStatus::Ok;
                    r.body = std::move(body);
                    cb(std::move(r));
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "CanteensService::menu: " << e.base().what();
                    MenuResult r;
                    r.status = MenuStatus::Error;  // fail-closed → 500
                    cb(std::move(r));
                };
        });
}

}  // namespace para
