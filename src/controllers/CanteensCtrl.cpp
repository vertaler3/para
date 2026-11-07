#include "controllers/CanteensCtrl.h"

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
