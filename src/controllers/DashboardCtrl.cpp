#include "controllers/DashboardCtrl.h"

#include <optional>
#include <string>

#include "services/DashboardService.h"
#include "utils/ApiError.h"
#include "utils/I18nView.h"

using namespace drogon;
using namespace para;

namespace {

// Сессии нет (нет cookie / невалидна) → /login. 303 See Other — редирект
// SSR-страниц по 02 §6.3 («нет сессии → 303 /login (SSR)»); синтаксис
// v1.9.14 — newRedirectionResponse(location, status) (в v1.9.14 нет
// newHttpRedirectResponse — проверено сборкой).
HttpResponsePtr loginRedirect()
{
    return HttpResponse::newRedirectionResponse("/login", k303SeeOther);
}

}  // namespace

void DashboardCtrl::page(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Путь «/» LoginFilter пропускает как публичный (LoginFilter.cpp:
    // isPublic → path == "/") — сессию проверяет контроллер сам.
    const std::string token = req->getCookie(SessionService::kCookieName);
    if (token.empty())
    {
        cb(loginRedirect());
        return;
    }
    SessionService::findValid(
        app().getDbClient("para"), token,
        [cb = std::move(cb),
         lang = i18n::resolveRequestLang(req)](std::optional<SessionInfo> session) mutable {
            if (!session)
            {
                cb(loginRedirect());
                return;
            }
            // В view-данные — серверные константы и флаг staffEntry;
            // контент виджетов грузит static/js/app.js из /v1/dashboard,
            // пользовательских данных в шаблоне нет — экранировать
            // нечего (02 §6.4, как views/login.csp).
            // 5.18 (шаг 3): строки словаря — ДО рендера (Р-16), язык
            // резолвится по запросу (para_lang → Accept-Language → ru).
            HttpViewData data;
            i18n::insertUiStrings(data, lang);
            data.insert("pageTitle", i18n::pageTitleOf("nav.today", lang));
            data.insert("activeTab", std::string("today"));
            // Точка входа сотрудника (фикс ревью экрана «Стол
            // сотрудника» — сотрудник после входа попадает сюда, и без
            // флага /staff недостижим из навигации). Роль — из сессии
            // сервера, не от клиента (02 §6.3); флаг читают
            // views/dashboard.csp (карточка-вход) и partials/topbar.csp
            // (ссылка в шапке). Студенту флага нет — ни карточки, ни
            // ссылки: 403-карточка стола студенту не показывается.
            if (session->role == "staff")
                data.insert("staffEntry", std::string("1"));
            cb(HttpResponse::newHttpViewResponse("dashboard",
                                                 std::move(data)));
        });
}

void DashboardCtrl::aggregate(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    // Логика агрегации — DashboardService (AGENTS §5.5); здесь только
    // маппинг результата: ok → 200 JSON, иначе 500 (формат 02 §5.1,
    // без деталей — 02 §5.1: 500 — неожиданное, без стека наружу).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    DashboardService::dashboard(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](DashboardService::Result r) mutable {
            if (r.ok)
                cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal",
                                      "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}
