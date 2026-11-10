#include "controllers/LibraryCtrl.h"

#include <drogon/drogon.h>

#include "services/LibraryService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат Result-сервисов → ответ: ok — JSON (api::jsonResponse:
// кириллица сырыми UTF-8 байтами — гейт-конвенция Э6 р1), иначе единый
// формат ошибки 02 §5.1 (500 — неожиданное, без деталей).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                LibraryService::Result r)
{
    if (r.ok)
        cb(api::jsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

}  // namespace

void LibraryCtrl::getBranches(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — каталог общий, user-скоупа нет
    // (02 §6.3; шапка LibraryService).
    LibraryService::branches(
        app().getDbClient("para"),
        [cb = std::move(cb)](LibraryService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void LibraryCtrl::getItems(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // query контракта 02 §5.2: q (подстрока title/author), branch_id
    // («не целое → 400» — стиль capacity в RoomsCtrl), available=1
    // («свободно сейчас»; иное значение → 400 — стиль upcoming).
    LibraryService::ItemFilters filters;
    filters.q = req->getParameter("q");
    const auto branch = req->getOptionalParameter<int64_t>("branch_id");
    if (branch)
    {
        filters.branchId = *branch;
    }
    else if (!req->getParameter("branch_id").empty())
    {
        cb(api::errorResponse("bad_request",
                              "branch_id — ожидается целое число",
                              k400BadRequest));
        return;
    }
    const std::string rawAvailable = req->getParameter("available");
    if (rawAvailable == "1")
        filters.availableOnly = true;
    else if (!rawAvailable.empty())
    {
        cb(api::errorResponse("bad_request", "available — ожидается 1",
                              k400BadRequest));
        return;
    }
    LibraryService::items(
        app().getDbClient("para"), filters,
        [cb = std::move(cb)](LibraryService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void LibraryCtrl::getMyCard(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3):
    // билет всегда СВОЙ, чужого билета по адресу не существует (path-id нет).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    LibraryService::myCard(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](LibraryService::CardResult r) mutable {
            switch (r.status)
            {
                case LibraryService::CardStatus::Ok:
                    cb(api::jsonResponse(std::move(r.body)));
                    return;
                case LibraryService::CardStatus::NotFound:
                    cb(api::errorResponse("not_found",
                                          "Читательский билет не найден",
                                          k404NotFound));
                    return;
                case LibraryService::CardStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

// --- Страничная плоскость (Э7: GET /library; см. шапку заголовка) ---------

void LibraryCtrl::libraryPage(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 /login —
    // правка фильтра Э7/Р-19, как RoomsCtrl::roomsPage). Тонкая страница
    // без данных (AGENTS §5.5): филиалы/наличие грузит клиент из тех же
    // JSON-роутов этого контроллера (GET /v1/library/branches, GET
    // /v1/library/items — 02 §5.2); query контракта (?q=, ?branch_id=,
    // ?available=1) — тоже на клиенте (fetch). Состояния 03 §5 — в
    // шаблоне и static/js/app.js.
    (void)req;
    HttpViewData data;
    data.insert("pageTitle", std::string("Библиотека · Пара"));
    // Экран НЕ таб (03 §3.1 — вход из профиля, 03 §4.9.6/5.12): пустой
    // activeTab — ни один пункт таб-бара не получает aria-current.
    data.insert("activeTab", std::string(""));
    cb(HttpResponse::newHttpViewResponse("library", std::move(data)));
}
