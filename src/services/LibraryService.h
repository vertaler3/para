#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Библиотека (5.12; Э6б шаг 2; 02 §5.2: GET /v1/library/branches —
// «филиалы библиотеки (корпус)», GET /v1/library/items — «наличие
// литературы: поиск, фильтр по филиалу и „свободно сейчас“, query: q,
// branch_id, available=1», GET /v1/library/my-card — «онлайн читательский
// билет (данные для QR), 200 · 404 (нет билета)»; таблицы
// library_branches/library_items/library_cards — миграция 0006, индекс
// idx_library_items_branch (branch_id, available) — 02 §4.2).
// Весь SQL — параметризованный и живёт здесь, не в контроллерах
// (AGENTS.md §5.5, 02 §6.4).
//
// Каталоги филиалов и книг — ОБЩИЕ ресурсы (как аудитории Э5а): OwnershipGuard
// не применяется, user-скоупа в выборках нет; личный объект здесь один —
// читательский билет, и он всегда выбирается по user_id СЕССИИ
// (02 §6.3 уровень 2: «списки всегда WHERE user_id = :session_user»),
// path-id у my-card нет — guard не нужен.
//
// Правило видимости 5.11–5.16 (02 §6.2-конец) на библиотеку не добавляет
// ограничений: наличие литературы — публичный каталог вуза (вымышленный);
// my-card отдаёт СВОИ данные (card_no + ФИО/группа из собственного
// профиля) — чужих данных в ответе нет по построению.
class LibraryService
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД → 500 контроллером (fail-closed)
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/library/branches → {"branches": [{id, name, building_id,
    // building}, ...]} — филиалы с корпусом (02 §5.2 «(корпус)»), по id
    // (ORDER BY id — стиль каталога RoomsService::catalog). Пустая таблица —
    // 200 с пустым branches (заполнение — фикстуры, не ошибка выборки).
    static void branches(drogon::orm::DbClientPtr db, ResultCb cb);

    // GET /v1/library/items → {"items": [...]} — наличие литературы,
    // сортировка по title (постановка шага 2). Поля: id/title/author/kind/
    // branch_id/branch (имя филиала)/building (корпус филиала)/hall/
    // total/available; «свободно N из M» FE считает из total/available.
    // Query-фильтры 02 §5.2 — «filters-or-empty» (одна фиксированная
    // параметризованная SQL, паттерн kQueueSql ApplicationsService; порядок
    // первых вхождений $N в тексте = порядку биндинга — правило Э5 шаг 2,
    // повторных вхождений нет):
    //   q            — подстрока по title ИЛИ author (LIKE '%q%', ESCAPE
    //                  '\': спецсимволы % _ \ в q экранирует сервис —
    //                  поиск по подстроке, не по шаблону); пустая строка =
    //                  без фильтра;
    //   branchId     — 0 = без фильтра, иначе branch_id = $N;
    //   availableOnly— false = без фильтра, true (query available=1) —
    //                  только available > 0 («свободно сейчас»).
    // LIKE в SQLite без ICU регистрозависим для не-ASCII (кириллицы) —
    // ограничение задокументировано в журнале шага; для демо-каталога поиск
    // работает по точному регистру подстроки.
    struct ItemFilters
    {
        std::string q;             // query q, пустая = без фильтра
        int64_t branchId = 0;      // query branch_id, 0 = без фильтра
        bool availableOnly = false;  // query available=1
    };
    static void items(drogon::orm::DbClientPtr db,
                      const ItemFilters& filters,
                      ResultCb cb);

    // GET /v1/library/my-card → 200 {card_no, fio, group} — билет ТЕКУЩЕГО
    // пользователя (данные для QR, 5.12): card_no из library_cards, ФИО и
    // группа — из СВОЕГО профиля (LEFT JOIN: у staff билета без профиля
    // fio/group — null; связка Р-14 по email — конвенция PeopleCtrl, в
    // my-card сознательно не тащится). Нет билета → NotFound (404
    // контроллером, 02 §5.2 «404 (нет билета)»); ошибка БД → Error
    // (fail-closed 500).
    enum class CardStatus
    {
        Ok,
        NotFound,
        Error
    };
    struct CardResult
    {
        CardStatus status = CardStatus::Error;
        Json::Value body;  // {card_no, fio, group} при Ok
    };
    static void myCard(drogon::orm::DbClientPtr db,
                       int64_t userId,
                       std::function<void(CardResult)> cb);
};

}  // namespace para
