#pragma once

#include <functional>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Столовые (5.13; Э6б шаг 3; 02 §5.2: GET /v1/canteens — «список столовых:
// корпус, часы», GET /v1/canteens/{id}/menu — «меню: цены, комплексные,
// КБЖУ, 200 · 404»; таблицы canteens/menu_items — миграция 0006, индекс
// idx_menu_items_canteen (canteen_id, category) — 02 §4.2). Весь SQL —
// параметризованный и живёт здесь, не в контроллерах (AGENTS.md §5.5,
// 02 §6.4). Каталог — общий ресурс (OwnershipGuard не применяется, как
// библиотека/аудитории); path-id есть, но меню — публичный каталог, а не
// личный объект (правило guard'а — личные данные, шапка OwnershipGuard.h).
class CanteensService
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД → 500 контроллером (fail-closed)
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/canteens → {"canteens": [{id, name, building_id, building,
    // hours}, ...]} — JOIN buildings (корпус, 02 §5.2), ORDER BY id (стиль
    // каталогов). Пустая таблица — 200 с пустым массивом.
    static void list(drogon::orm::DbClientPtr db, ResultCb cb);

    // Существование столовой (404 контроллера). Found/NotFound/Error —
    // паттерн RoomsService::roomById.
    enum class Lookup
    {
        Found,
        NotFound,
        Error
    };
    static void canteenById(
        drogon::orm::DbClientPtr db,
        int64_t canteenId,
        std::function<void(Lookup, Json::Value canteen)> cb);

    // GET /v1/canteens/{id}/menu → 200 {"canteen_id", "canteen",
    // "building", "hours", "menu": [{id, name, category, is_combo, price,
    // kcal, protein, fat, carbs}, ...]} — цены в ₽, is_combo — bool
    // («комплексные»), КБЖУ nullable (water без КБЖУ — null, fixtures Э6б).
    // Пустое меню существующей столовой — 200 с пустым menu (не ошибка
    // выборки). NotFound — столовой нет (404, 02 §5.2); Error — БД.
    enum class MenuStatus
    {
        Ok,
        NotFound,
        Error
    };
    struct MenuResult
    {
        MenuStatus status = MenuStatus::Error;
        Json::Value body;
    };
    static void menu(drogon::orm::DbClientPtr db,
                     int64_t canteenId,
                     std::function<void(MenuResult)> cb);
};

}  // namespace para
