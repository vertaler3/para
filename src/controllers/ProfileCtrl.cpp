#include "controllers/ProfileCtrl.h"

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter
#include "services/IcsService.h"
#include "utils/I18nView.h"

using namespace drogon;
using namespace para;

void ProfileCtrl::page(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 → /login,
    // Р-20); в view-данные — только серверные константы (заголовок
    // вкладки, активный таб), как DashboardCtrl::page. 5.18 (шаг 3):
    // строки словаря — в view-данные ДО рендера (Р-16), язык — para_lang →
    // Accept-Language → ru (включая сегмент языка секции «Оформление»).
    // fe_rebuild шаг 16: флаг icsState «active|none|unknown» — есть ли
    // активная подписка (IcsService::activeToken, тот же источник, что
    // GET /v1/ics/token). Сам токен в HTML не попадает: при active блок
    // забирает ссылку GET-ом (200), при none — сразу «Создать подписку»
    // без заведомого 404 в консоли; unknown (сбой БД) — клиент спросит сам.
    const auto lang = i18n::resolveRequestLang(req);
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    IcsService::activeToken(
        app().getDbClient("para"), session.userId,
        [lang, cb = std::move(cb)](IcsService::ActiveToken t) mutable {
            HttpViewData data;
            i18n::insertUiStrings(data, lang);
            data.insert("pageTitle", i18n::pageTitleOf("nav.profile", lang));
            data.insert("activeTab", std::string("profile"));
            data.insert("icsState", std::string(!t.ok ? "unknown" : (t.found ? "active" : "none")));
            cb(HttpResponse::newHttpViewResponse("profile", std::move(data)));
        });
}
