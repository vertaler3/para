#include "controllers/AssistantCtrl.h"

#include <ctime>

#include "services/AssistantService.h"
#include "services/SessionService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void AssistantCtrl::chat(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");

    std::string message;
    const auto json = req->getJsonObject();
    switch (AssistantService::validate(json.get(), message))
    {
        case AssistantService::Input::Empty:
            cb(api::errorResponse("bad_request",
                                  "Напишите вопрос ассистенту",
                                  k400BadRequest));
            return;
        case AssistantService::Input::TooLong:
            cb(api::errorResponse("bad_request",
                                  "Вопрос длиннее 700 символов",
                                  k400BadRequest));
            return;
        case AssistantService::Input::Ok:
            break;
    }

    if (const int64_t wait = AssistantService::RateLimiter::instance().acquire(
            session.userId, static_cast<int64_t>(std::time(nullptr)));
        wait > 0)
    {
        auto resp = api::errorResponse("rate_limited",
                                       "Слишком много сообщений — "
                                       "подождите минуту",
                                       k429TooManyRequests);
        resp->addHeader("Retry-After", std::to_string(wait));
        cb(resp);
        return;
    }

    AssistantService::chat(
        app().getDbClient("para"), session, message,
        AssistantService::configFromEnv(), AssistantService::httpTransport(),
        [cb = std::move(cb)](AssistantService::ChatResult r) mutable {
            switch (r.status)
            {
                case AssistantService::Status::Ok:
                {
                    Json::Value body(Json::objectValue);
                    body["reply"] = std::move(r.reply);
                    cb(api::jsonResponse(std::move(body)));
                    return;
                }
                case AssistantService::Status::Unavailable:
                    cb(api::errorResponse("assistant_unavailable",
                                          "Ассистент сейчас недоступен — "
                                          "попробуйте чуть позже",
                                          k502BadGateway));
                    return;
                case AssistantService::Status::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

void AssistantCtrl::history(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    AssistantService::history(
        app().getDbClient("para"), session.userId,
        AssistantService::kHistoryForClient,
        [cb = std::move(cb)](bool ok, Json::Value messages) mutable {
            if (!ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            Json::Value body(Json::objectValue);
            body["messages"] = std::move(messages);
            cb(api::jsonResponse(std::move(body)));
        });
}
