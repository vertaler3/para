#include "controllers/CanteensCtrl.h"

#include "utils/I18nView.h"
#include <drogon/drogon.h>

#include "services/CanteensService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void CanteensCtrl::getCanteens(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — каталог общий, user-скоупа нет
    // (02 §6.3; шапка CanteensService).
    CanteensService::list(
        app().getDbClient("para"),
        [cb = std::move(cb)](CanteensService::Result r) mutable {
            if (r.ok)
                cb(api::jsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}

void CanteensCtrl::getMenu(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t canteenId)
{
    // {id} — публичная столовая (не личный объект): OwnershipGuard не
    // применяется (шапка CanteensService); несуществующая → 404 (02 §5.2).
    CanteensService::menu(
        app().getDbClient("para"), canteenId,
        [cb = std::move(cb)](CanteensService::MenuResult r) mutable {
            switch (r.status)
            {
                case CanteensService::MenuStatus::Ok:
                    cb(api::jsonResponse(std::move(r.body)));
                    return;
                case CanteensService::MenuStatus::NotFound:
                    cb(api::errorResponse("not_found", "Столовая не найдена",
                                          k404NotFound));
                    return;
                case CanteensService::MenuStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

// --- Страничная плоскость (Э7: GET /canteens; см. шапку заголовка) --------

void CanteensCtrl::canteensPage(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 /login —
    // правка фильтра Э7/Р-19, как RoomsCtrl::roomsPage). Тонкая страница
    // без данных (AGENTS §5.5): столовые/меню грузит клиент из тех же
    // JSON-роутов этого контроллера (GET /v1/canteens, GET
    // /v1/canteens/{id}/menu — 02 §5.2). Состояния 03 §5 — в шаблоне и
    // static/js/app.js.
    // 5.18 (шаг 3): каркас — словарь из view-данных (Р-16); fe_rebuild
    // шаг 20: тело экрана и заголовок вкладки тоже на словаре (canteens.*).
    HttpViewData data;
    const auto lang = i18n::resolveRequestLang(req);
    i18n::insertUiStrings(data, lang);
    data.insert("pageTitle", i18n::pageTitleOf("campus.canteen", lang));
    // Экран НЕ таб (03 §3.1 — вход из профиля, 03 §4.9.6/5.13): пустой
    // activeTab — ни один пункт таб-бара не получает aria-current.
    data.insert("activeTab", std::string(""));
    cb(HttpResponse::newHttpViewResponse("canteens", std::move(data)));
}
