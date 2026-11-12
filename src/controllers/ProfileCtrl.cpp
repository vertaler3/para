#include "controllers/ProfileCtrl.h"

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
    HttpViewData data;
    const auto lang = i18n::resolveRequestLang(req);
    i18n::insertUiStrings(data, lang);
    data.insert("pageTitle", i18n::pageTitleOf("nav.profile", lang));
    data.insert("activeTab", std::string("profile"));
    cb(HttpResponse::newHttpViewResponse("profile", std::move(data)));
}
