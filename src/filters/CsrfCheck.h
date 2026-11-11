#pragma once

#include <drogon/Cookie.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>

#include <cstdint>
#include <optional>
#include <string>

namespace para
{
// CSRF double-submit (02 §6.4 дословно): «токен double-submit (cookie csrf +
// скрытое поле/заголовок, проверка в CsrfCheck) …; state-changing только
// POST/PUT/DELETE». Реализовано шагом 2 Э8.
//
// Механика:
//   • Токен детерминированно привязан к сессии: sha256("para-csrf-v1:" +
//     токен сессии) → 64 hex (256 бит). Это «signed double-submit» (OWASP
//     CSRF Cheat Sheet): подменить/подкинуть cookie csrf недостаточно —
//     верное значение не вычислить без HttpOnly-cookie сессии. Серверное
//     состояние не хранится вовсе: токен перевычисляется из сессии в любой
//     момент (переживает рестарт, не требует чистки).
//   • Выдача: Set-Cookie csrf при входе (AuthCtrl::login) и на каждой
//     странице (issueToPageAdvice — post-handling advice, main.cpp; тот же
//     вызов в security_tests — сервер в тесте идентичен продовому). Cookie
//     БЕЗ HttpOnly (токен читает static/js/app.js — единственная cookie
//     проекта без этого флага; секрет сессии остаётся HttpOnly);
//     Secure; SameSite=Lax; Max-Age = TTL сессии. Плюс <meta
//     name="csrf-token"> в <head> каждой страницы (канон htmx).
//   • Проверка — verify(): submitted (заголовок X-CSRF-Token, иначе поле
//     формы _csrf) == cookie csrf == tokenFor(токен сессии); иначе 403 в
//     формате 02 §5.1, error.code="csrf" («страница устарела — обновите»).
//     Вызывается из LoginFilter ПОСЛЕ валидации сессии на всех приватных
//     POST/PUT/DELETE — ЕДИНАЯ точка вызова покрывает все текущие и будущие
//     роуты под фильтром (правило 02 §6.3 «новые приватные роуты обязаны
//     добавляться с LoginFilter»; отдельный фильтр на каждый роут дал бы
//     дрейф). Отдельный компонент имени CsrfCheck из 02 §6.4 — этот класс.
//   • Вне проверки по спеке 02 §6.4: GET — чтение («state-changing только
//     POST/PUT/DELETE»), /ws — GET-handshake. POST /v1/auth/login — до
//     сессии токена не существует: вход остаётся за SameSite=Lax + лимитером
//     (тот же §6.4); смок AGENTS §4 сохранён — решение в журнале Э8 шага 2.
//   • htmx-фрагменты GET (ScheduleCtrl::segment, StaffCtrl::appCardFragment)
//     уже требуют HX-Request с Э7 (02 §6.4 «требование заголовка HX-Request
//     для htmx-эндпоинтов») — state-changing htmx-эндпоинтов в проекте нет.
class CsrfCheck
{
  public:
    static constexpr const char* kCookieName = "csrf";
    static constexpr const char* kHeaderName = "X-CSRF-Token";
    static constexpr const char* kFormField = "_csrf";

    // Токен CSRF сессии (64 hex, 256 бит) — sha256 от доменно-разделённого
    // токена сессии (SessionService::generateToken, CSPRNG OpenSSL).
    static std::string tokenFor(const std::string& sessionToken);

    // Cookie csrf для ответа: без HttpOnly (читает app.js), Secure,
    // SameSite=Lax, Path=/, Max-Age = TTL сессии.
    static drogon::Cookie cookieFor(const std::string& sessionToken,
                                    int64_t ttlSec);

    // Проверка state-changing запроса (Post/Put/Delete). nullopt — пройдена;
    // иначе готовый 403 (формат 02 §5.1, error.code="csrf").
    static std::optional<drogon::HttpResponsePtr> verify(
        const drogon::HttpRequestPtr& req);

    // Регистрация post-handling advice «страница выдаёт токен»: Set-Cookie
    // csrf, когда расходится с сессией (самовосстановление после удаления
    // cookie — без повторного входа), и <meta name="csrf-token"> в <head>
    // каждой HTML-страницы (head.csp — единственный </head> в теле;
    // htmx-фрагменты без </head> мету не получают — им не нужно: htmx ходит
    // только GET). Один вызов на процесс: main.cpp и security_tests.
    static void registerPageIssuanceAdvice();
};

}  // namespace para
