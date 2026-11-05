#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/HttpAppFramework.h>
#include <drogon/orm/DbClient.h>

namespace para {

// Сессии — собственный вариант 02 §6.1: cookie со случайным 128-бит
// токеном → таблица sessions (переживают рестарт бинаря); встроенные
// сессии Drogon (enable_session) НЕ используются.
// Выдача cookie — единый хелпер sessionCookie() с флагами
// HttpOnly; Secure; SameSite=Lax (02 §6.1).
struct SessionInfo
{
    int64_t userId = 0;
    std::string login;
    std::string role;
};

class SessionService
{
  public:
    static constexpr const char* kCookieName = "para_session";
    // Таймаут с запасом на весь период оценивания (итоги — 28.11.2026):
    // 30 дней от входа; скользящее продление при активности — Э2 (02 §6.1).
    static constexpr int64_t kDefaultTtlSec = 30 * 24 * 3600;

    // Действующий TTL: PARA_SESSION_TTL_HOURS (env, часы), иначе дефолт.
    // Секретов нет — только число (AGENTS.md §5.2).
    static int64_t currentTtlSec();

    // Заголовок Set-Cookie со всеми флагами (единый хелпер, 02 §6.1).
    static std::string sessionCookie(const std::string& token, int64_t ttlSec);

    // Случайный токен в hex (CSPRNG OpenSSL): bytes байт энтропии →
    // bytes*2 hex-символов. Дефолт 16 байт = 128 бит — токен сессии
    // (02 §6.1, Р-7); приватный токен подписки ICS берёт 32 байта =
    // 256 бит (64 hex-символа) — Р-11 (правки по гейту Э3, раунд 1).
    static std::string generateToken(std::size_t bytes = 16);

    // Создать сессию (заодно чистит истёкшие — idx_sessions_expiry, 02 §4.2).
    static void create(drogon::orm::DbClientPtr db,
                       int64_t userId,
                       int64_t ttlSec,
                       std::function<void(bool ok, const std::string& token)> cb);

    // Валидная (не истёкшая) сессия по токену; cb(optional).
    static void findValid(drogon::orm::DbClientPtr db,
                          const std::string& token,
                          std::function<void(std::optional<SessionInfo>)> cb);

    // Удалить сессию (logout — инвалидация на сервере, 02 §6.1).
    static void remove(drogon::orm::DbClientPtr db,
                       const std::string& token,
                       std::function<void()> cb);

    // Текущий момент в формате ISO 8601 UTC проекта (02 §4).
    static std::string utcNow();
};

}  // namespace para
