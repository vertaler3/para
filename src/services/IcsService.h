#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>

namespace para {

// Приватная подписка календаря (идея 5.4; 02 §5.2: POST/DELETE /v1/ics/token,
// GET /ics/{token}; таблица ics_tokens — миграция 0004: user_id — PRIMARY KEY,
// token UNIQUE, revoked DEFAULT 0 — один активный токен на пользователя
// закреплён схемой). Токен — случайный 256-бит hex (64 hex-символа) через
// CSPRNG OpenSSL (SessionService::generateToken(32) — тот же генератор, что
// у токена сессии, Р-7; длина — Р-11, правки по гейту Э3 раунд 1). GET
// авторизуется самим токеном (02 §6.1: /ics/{token} — без сессионной
// cookie); POST/DELETE — по сессии. Весь SQL — здесь (AGENTS.md §5.5,
// 02 §6.4); VCALENDAR — IcsBuilder.
class IcsService
{
  public:
    // POST /v1/ics/token → 201 {token}: создать или перевыпустить токен
    // (UPSERT по user_id — ротация: предыдущий токен перестаёт работать,
    // строка в таблице всегда одна).
    static void createToken(
        drogon::orm::DbClientPtr db,
        int64_t userId,
        std::function<void(bool ok, const std::string& token)> cb);

    // DELETE /v1/ics/token → 204: отозвать активный токен; идемпотентно —
    // отзыва без активного токена не ошибка.
    static void revokeToken(drogon::orm::DbClientPtr db,
                            int64_t userId,
                            std::function<void(bool ok)> cb);

    struct Calendar
    {
        bool ok = false;     // false — ошибка БД → контроллер 500
        bool found = false;  // false — токен неверный/отозван → 404
        std::string ics;     // тело VCALENDAR (при found)
    };

    // GET /ics/{token}: токен → владелец → расписание его группы (ВСЕ
    // статусы, включая moved/cancelled — календарь честный) + персональные
    // даты session_plan владельца → VCALENDAR (RFC 5545, CRLF).
    static void calendarByToken(drogon::orm::DbClientPtr db,
                                const std::string& token,
                                std::function<void(Calendar)> cb);

    // Текущий момент в формате ICS DATE-TIME (20261105T201830Z) — DTSTAMP.
    static std::string stampNow();
};

}  // namespace para
