#include "controllers/IcsCtrl.h"

#include "services/IcsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void IcsCtrl::getCalendar(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb,
                          std::string token)
{
    // Публичный путь: аутентификация — сам токен (02 §6.1). req не нужен.
    (void)req;
    IcsService::calendarByToken(
        app().getDbClient("para"),
        token,
        [cb = std::move(cb)](IcsService::Calendar c) mutable {
            if (!c.ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            if (!c.found)
            {
                // Неверный или отозванный токен → 404 (02 §5.2), формат
                // ошибки — единый 02 §5.1.
                cb(api::errorResponse("not_found", "Подписка не найдена",
                                      k404NotFound));
                return;
            }
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            // text/calendar через setContentTypeString (НЕ addHeader):
            // newHttpResponse ставит дефолтный text/html — addHeader
            // ДОБАВЛЯЛ бы второй Content-Type (дубль — malformed HTTP,
            // RFC 9110 §7.2; поймано E2E гейта, раунд 2).
            resp->setContentTypeString("text/calendar; charset=utf-8");
            resp->setBody(std::move(c.ics));
            cb(resp);
        });
}

void IcsCtrl::createToken(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    IcsService::createToken(
        app().getDbClient("para"),
        session.userId,
        [cb = std::move(cb)](bool ok, const std::string& token) mutable {
            if (!ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            Json::Value body;
            body["token"] = token;
            auto resp = HttpResponse::newHttpJsonResponse(std::move(body));
            resp->setStatusCode(k201Created);
            cb(resp);
        });
}

void IcsCtrl::revokeToken(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    IcsService::revokeToken(
        app().getDbClient("para"),
        session.userId,
        [cb = std::move(cb)](bool ok) mutable {
            if (!ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k204NoContent);
            cb(resp);
        });
}
