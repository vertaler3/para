#include "controllers/ProfileCtrl.h"

using namespace drogon;

void ProfileCtrl::page(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 → /login,
    // Р-20) — req не нужен; в view-данные — только серверные константы
    // (заголовок вкладки, активный таб), как DashboardCtrl::page.
    (void)req;
    HttpViewData data;
    data.insert("pageTitle", std::string("Профиль · Пара"));
    data.insert("activeTab", std::string("profile"));
    cb(HttpResponse::newHttpViewResponse("profile", std::move(data)));
}
