#include "controllers/SettingsCtrl.h"

#include "services/SettingsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void SettingsCtrl::get(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session =
        req->attributes()->get<para::SessionInfo>("session");
    SettingsService::load(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](SettingsRecord rec) {
            Json::Value body;
            SettingsService::toJson(rec, body);
            cb(HttpResponse::newHttpJsonResponse(std::move(body)));
        });
}

void SettingsCtrl::update(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session =
        req->attributes()->get<para::SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request",
                              "Ожидается JSON-тело настроек",
                              k400BadRequest));
        return;
    }
    // Загрузили текущую строку своей сессии → применили патч → UPSERT.
    SettingsService::load(
        app().getDbClient("para"), session.userId,
        [this, json, session, cb = std::move(cb)](SettingsRecord base) {
            SettingsRecord rec;
            rec.userId = session.userId;  // только из сессии (02 §6.3)
            const auto patch = SettingsService::applyPatch(base, *json, rec);
            if (!patch.ok)
            {
                cb(api::errorResponse(patch.errorCode, patch.errorMessage,
                                      k400BadRequest));
                return;
            }
            SettingsService::save(
                app().getDbClient("para"), rec,
                [rec, cb = std::move(cb)](bool ok) {
                    if (!ok)
                    {
                        cb(api::errorResponse("internal",
                                              "Внутренняя ошибка сервера",
                                              k500InternalServerError));
                        return;
                    }
                    Json::Value body;
                    SettingsService::toJson(rec, body);
                    cb(HttpResponse::newHttpJsonResponse(std::move(body)));
                });
        });
}
