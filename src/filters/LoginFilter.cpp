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

// API-плоскость (02 §3.2: JSON-данные и htmx-фрагменты — под /v1/; push-канал
// /ws — тоже API): без сессии отвечает 401 JSON — контракт гейта Э6
// («wsБезCookie» → 401) и всех /v1/*-проверок. СТРАНИЦА (SSR-HTML, 02 §3.2
// «SSR-страницы /, /login, /schedule, /grades …») — всё остальное, что не
// публично: без сессии браузеру отвечает 303 → /login (02 §6.3).
bool isApi(const std::string& path)
{
    return path.rfind("/v1/", 0) == 0 || path == "/ws";
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
    // Страница (SSR-HTML) или API — от этого зависит ответ без сессии:
    //   API (JSON /v1/* и WS-канал /ws) → 401 без деталей (02 §6.3); формат
    //     тела — единый (02 §5.1), message не раскрывает разницу «нет
    //     cookie» / «сессия невалидна» (проверки гейтов Э1–Э6 на нём стоят);
    //   страница (SSR-HTML) → 303 → /login (02 §6.3 «нет сессии → 303
    //     /login (SSR)», как GET / в DashboardCtrl) — браузер сам пройдёт
    //     редирект на форму входа.
    const bool api = isApi(req->path());
    const std::string token = req->getCookie(para::SessionService::kCookieName);
    if (token.empty())
    {
        if (api)
            fcb(para::api::errorResponse("unauthorized",
                                         "Требуется авторизация",
                                         k401Unauthorized));
        else
            fcb(HttpResponse::newRedirectionResponse("/login",
                                                     k303SeeOther));
        return;
    }
    para::SessionService::findValid(
        app().getDbClient("para"), token,
        [req, api, fcb = std::move(fcb),
         fccb = std::move(fccb)](std::optional<para::SessionInfo> session) {
            if (!session)
            {
                if (api)
                    fcb(para::api::errorResponse("unauthorized",
                                                 "Требуется авторизация",
                                                 k401Unauthorized));
                else
                    fcb(HttpResponse::newRedirectionResponse("/login",
                                                             k303SeeOther));
                return;
            }
            req->attributes()->insert("session", std::move(*session));
            fccb();
        });
}
