#pragma once

#include <drogon/HttpFilter.h>

#include "services/SessionService.h"

using namespace drogon;

// LoginFilter (02 §6.3): всё, кроме публичных путей, требует валидную
// сессию (таблица sessions, 02 §6.1). Без сессии ответ зависит от плоскости:
// API (/v1/* и push-канал /ws) — 401 JSON без деталей (02 §5.1); СТРАНИЦА
// (SSR-HTML — /schedule, /grades, …) — 303 redirect → /login (02 §6.3:
// «нет сессии → 303 /login (SSR)»; Э7, LoginFilter.cpp isApi).
// Публичное: /healthz, POST /v1/auth/login, страница входа /login
// (CSP-шаблон), статика FE (/css, /js, /vendor, /img, /icons, /fonts,
// /static — префиксы по дереву 05 §2; точные пути статики согласует
// FE-дорожка на Э3) и /ics/{token} с Э3 шага 2 — приватная подписка
// календаря 5.4: аутентификация самим секретным токеном (02 §6.1: «…кроме
// /v1/auth/login, /ics/{token}… и /healthz»); GET /ics/{token} работает
// без сессии, POST/DELETE /v1/ics/token — по сессии (роут с фильтром).
// Механика Drogon: глобальных фильтров нет — фильтр навешивается на роуты
// явно (PATH_ADD/METHOD_ADD ..., "LoginFilter"); новые приватные роуты Э2+
// обязаны добавляться с ним (правило код-ревью, 02 §6.3).
// CSRF double-submit на всех приватных state-changing — с шага 2 Э8:
// LoginFilter ПОСЛЕ валидации сессии вызывает CsrfCheck::verify на
// POST/PUT/DELETE (02 §6.4; единая точка — см. CsrfCheck.h). Публичный
// POST /v1/auth/login фильтром не проходится — вход до сессии, токена
// ещё нет (журнал Э8 шага 2); базовый барьер — SameSite=Lax cookie.
class LoginFilter : public HttpFilter<LoginFilter>
{
  public:
    void doFilter(const HttpRequestPtr& req,
                  FilterCallback&& fcb,
                  FilterChainCallback&& fccb) override;
};
