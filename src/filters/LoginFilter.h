#pragma once

#include <drogon/HttpFilter.h>

#include "services/SessionService.h"

using namespace drogon;

// LoginFilter (02 §6.3): всё, кроме публичных путей, требует валидную
// сессию (таблица sessions, 02 §6.1); без неё API отвечает 401 без деталей.
// Для SSR-страниц будет 303 → /login — с Э2 (02 §6.3), здесь только API.
// Публичное: /healthz, POST /v1/auth/login, страница входа /login
// (CSP-шаблон) и статика FE (/css, /js, /vendor, /img, /icons, /fonts,
// /static — префиксы по дереву 05 §2; точные пути статики согласует
// FE-дорожка на Э3).
// Механика Drogon: глобальных фильтров нет — фильтр навешивается на роуты
// явно (PATH_ADD/METHOD_ADD ..., "LoginFilter"); новые приватные роуты Э2+
// обязаны добавляться с ним (правило код-ревью, 02 §6.3).
// CSRF-проверка на state-changing — Э8 (02 §6.4); сейчас барьер —
// SameSite=Lax у cookie.
class LoginFilter : public HttpFilter<LoginFilter>
{
  public:
    void doFilter(const HttpRequestPtr& req,
                  FilterCallback&& fcb,
                  FilterChainCallback&& fccb) override;
};
