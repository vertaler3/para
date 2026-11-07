#include "services/LibraryService.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;  // Row, Result, DrogonDbException
// ВАЖНО (урок RoomsService.cpp:19): внутри тела методов LibraryService имя
// `Result` резолвится в para::LibraryService::Result (class scope), поэтому
// тип строк выборки в лямбдах пишется полностью — drogon::orm::Result.

namespace para {

namespace {

// Общий хвост: JOIN имени филиала и корпуса (02 §5.2: филиал и «корпус» в
// карточке книги; «свободно N из M» FE считает из total/available).
const std::string kItemJoins =
    " FROM library_items li"
    " JOIN library_branches lb ON lb.id = li.branch_id"
    " JOIN buildings b ON b.id = lb.building_id";

// Query-фильтры 02 §5.2 в стиле «filters-or-empty» (kQueueSql
// ApplicationsService): фиксированная SQL, все фильтры параметрами.
// Порядок первых вхождений $N в тексте = порядку биндинга (правило
// Э5 шаг 2: «$N — имя параметра с индексом по первому вхождению, drogon
// биндит аргументы позиционно») — повторных вхождений нет:
//   $1 q-флаг ('' = без фильтра), $2/$3 — LIKE-паттерн для title/author,
//   $4/$5 — branch_id (0 = без фильтра), $6 — available-флаг (1 = только
//   available > 0). LIKE-паттерн готовит контроллер-сервис ниже.
// available-ветка $6 = 0/1 имплицирует условие индекса
// idx_library_items_branch (branch_id, available) при фильтре по филиалу
// (02 §4.2 — «каталоги библиотеки», Э6б); сортировка — по title
// (постановка шага 2).
const std::string kItemsSql =
    "SELECT li.id, li.title, li.author, li.kind, li.branch_id,"
    " lb.name AS branch, b.name AS building, li.hall, li.total,"
    " li.available" +
    kItemJoins +
    " WHERE ($1 = '' OR li.title LIKE $2 ESCAPE '\\' OR"
    "        li.author LIKE $3 ESCAPE '\\')"
    "   AND ($4 = 0 OR li.branch_id = $5)"
    "   AND ($6 = 0 OR li.available > 0)"
    " ORDER BY li.title ASC";

// Экранирование спецсимволов LIKE (% _ \) в q: пользовательский ввод —
// подстрочный поиск, а не шаблон (ESCAPE '\' в kItemsSql). SQL-инъекции
// здесь нет и без этого (параметры), экранирование — про семантику поиска.
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

Json::Value itemJson(const Row& r)
{
    Json::Value it;
    it["id"] = r["id"].as<int64_t>();
    it["title"] = r["title"].as<std::string>();
    if (r["author"].isNull())
        it["author"] = Json::Value();
    else
        it["author"] = r["author"].as<std::string>();
    it["kind"] = r["kind"].as<std::string>();
    it["branch_id"] = r["branch_id"].as<int64_t>();
    it["branch"] = r["branch"].as<std::string>();
    it["building"] = r["building"].as<std::string>();
    if (r["hall"].isNull())
        it["hall"] = Json::Value();
    else
        it["hall"] = r["hall"].as<std::string>();
    it["total"] = r["total"].as<int64_t>();
    it["available"] = r["available"].as<int64_t>();
    return it;
}

}  // namespace

void LibraryService::branches(drogon::orm::DbClientPtr db, ResultCb cb)
{
    // Филиалы с корпусом (02 §5.2 «филиалы библиотеки (корпус)»): JOIN
    // buildings — название корпуса для строки списка. ORDER BY id — стиль
    // каталога аудиторий.
    *db << "SELECT lb.id, lb.name, lb.building_id, b.name AS building"
           " FROM library_branches lb JOIN buildings b ON b.id ="
           " lb.building_id ORDER BY lb.id ASC" >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
            {
                Json::Value br;
                br["id"] = r["id"].as<int64_t>();
                br["name"] = r["name"].as<std::string>();
                br["building_id"] = r["building_id"].as<int64_t>();
                br["building"] = r["building"].as<std::string>();
                arr.append(std::move(br));
            }
            Json::Value body;
            body["branches"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "LibraryService::branches: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void LibraryService::items(drogon::orm::DbClientPtr db,
                           const ItemFilters& filters,
                           ResultCb cb)
{
    // Пустая строка/0 = «без фильтра» (filters-or-empty в kItemsSql);
    // LIKE-паттерн готовится один раз — $2/$3, спецсимволы q экранированы.
    const std::string qFlag = filters.q.empty() ? "" : "1";
    const std::string qLike = "%" + escapeLike(filters.q) + "%";
    *db << kItemsSql << qFlag << qLike << qLike << filters.branchId
        << filters.branchId << (filters.availableOnly ? 1 : 0) >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
                arr.append(itemJson(r));
            Json::Value body;
            body["items"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "LibraryService::items: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void LibraryService::myCard(drogon::orm::DbClientPtr db,
                            int64_t userId,
                            std::function<void(CardResult)> cb)
{
    // Билет ТЕКУЩЕГО пользователя — строго по user_id сессии (02 §6.3
    // уровень 2; path-id нет — guard не нужен). LEFT JOIN профиля и группы:
    // у staff-билета (профиль не заводится — 02 §4.1) fio/group — null;
    // связка Р-14 (email = login||'@example.edu') в my-card сознательно не
    // тащится — ФИО сотрудника не часть данных QR-билета демо.
    *db << "SELECT c.card_no, p.fio AS fio, g.name AS group_name"
           " FROM library_cards c"
           " LEFT JOIN profiles p ON p.user_id = c.user_id"
           " LEFT JOIN groups g ON g.id = p.group_id"
           " WHERE c.user_id = $1"
        << userId >>
        [cb](const drogon::orm::Result& rows) {
            CardResult r;
            if (rows.empty())
            {
                r.status = CardStatus::NotFound;  // 404 (нет билета — 02 §5.2)
                cb(std::move(r));
                return;
            }
            const auto& row = rows[0];
            Json::Value card;
            card["card_no"] = row["card_no"].as<std::string>();
            if (row["fio"].isNull())
                card["fio"] = Json::Value();
            else
                card["fio"] = row["fio"].as<std::string>();
            if (row["group_name"].isNull())
                card["group"] = Json::Value();
            else
                card["group"] = row["group_name"].as<std::string>();
            r.status = CardStatus::Ok;
            r.body = std::move(card);
            cb(std::move(r));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "LibraryService::myCard: " << e.base().what();
            CardResult r;
            r.status = CardStatus::Error;  // fail-closed → 500
            cb(std::move(r));
        };
}

}  // namespace para
