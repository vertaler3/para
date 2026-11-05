#include "filters/LoginFilter.h"

#include "utils/ApiError.h"

namespace {

bool isPublic(const std::string& path)
{
    // Публичные префиксы — см. комментарий в LoginFilter.h. /ics/ —
    // приватная подписка календаря 5.4: авторизация самим секретным токеном
    // (02 §6.1: «Аутентификация всех эндпоинтов, кроме /v1/auth/login,
    // /ics/{token} … и /healthz, — через сессионную cookie и LoginFilter»).
    static const char* kPublicPrefixes[] = {
        "/css/",  "/js/",  "/vendor/", "/img/", "/icons/", "/fonts/",
        "/static/", "/healthz", "/v1/auth/login", "/login", "/ics/",
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
        // 401 без деталей (02 §6.3); формат тела — единый (02 §5.1), message
        // не раскрывает разницу «нет cookie» / «сессия невалидна».
        fcb(para::api::errorResponse("unauthorized",
                                     "Требуется авторизация",
                                     k401Unauthorized));
        return;
    }
    para::SessionService::findValid(
        app().getDbClient("para"), token,
        [req, fcb = std::move(fcb),
         fccb = std::move(fccb)](std::optional<para::SessionInfo> session) {
            if (!session)
            {
                fcb(para::api::errorResponse("unauthorized",
                                             "Требуется авторизация",
                                             k401Unauthorized));
                return;
            }
            req->attributes()->insert("session", std::move(*session));
            fccb();
        });
}
