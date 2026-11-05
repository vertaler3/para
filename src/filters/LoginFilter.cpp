#include "filters/LoginFilter.h"

namespace {

bool isPublic(const std::string& path)
{
    // Публичные префиксы — см. комментарий в LoginFilter.h.
    static const char* kPublicPrefixes[] = {
        "/css/",  "/js/",  "/vendor/", "/img/", "/icons/",
        "/fonts/", "/static/", "/healthz", "/v1/auth/login", "/login",
    };
    for (const char* prefix : kPublicPrefixes)
    {
        if (path.rfind(prefix, 0) == 0)
            return true;
    }
    return path == "/";
}

}  // namespace

void LoginFilter::doFilter(const HttpRequestPtr& req,
                           FilterCallback&& fcb,
                           FilterChainCallback&& fccb)
{
    if (isPublic(req->path()))
    {
        fccb();
        return;
    }
    const std::string token = req->getCookie(para::SessionService::kCookieName);
    if (token.empty())
    {
        // 401 без деталей (02 §6.3).
        auto resp = HttpResponse::newHttpJsonResponse(
            [] {
                Json::Value body;
                body["error"] = "unauthorized";
                return body;
            }());
        resp->setStatusCode(k401Unauthorized);
        fcb(resp);
        return;
    }
    para::SessionService::findValid(
        app().getDbClient("para"), token,
        [req, fcb = std::move(fcb),
         fccb = std::move(fccb)](std::optional<para::SessionInfo> session) {
            if (!session)
            {
                auto resp = HttpResponse::newHttpJsonResponse([] {
                    Json::Value body;
                    body["error"] = "unauthorized";
                    return body;
                }());
                resp->setStatusCode(k401Unauthorized);
                fcb(resp);
                return;
            }
            req->attributes()->insert("session", std::move(*session));
            fccb();
        });
}
