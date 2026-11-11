#pragma once

#include <drogon/HttpResponse.h>

namespace para
{
// Заголовки безопасности на ВСЕХ ответах сервера (02 §6.4; Э8 шаг 3):
//   • X-Content-Type-Options: nosniff — браузер не подменяет MIME;
//   • X-Frame-Options: DENY — фрейминг запрещён и там, где CSP
//     frame-ancestors ещё не поддержан (старые браузеры);
//   • Content-Security-Policy: default-src 'self' + inline ровно для того,
//     что реально inline в проекте, БЕЗ unsafe-eval (02 §6.4 дословно:
//     «default-src 'self'», «frame-ancestors 'none'», «никаких внешних
//     CDN — и безопасность, и доступность из РФ совпадают»);
//   • Referrer-Policy: strict-origin-when-cross-origin (02 §6 Referrer-
//     Policy не задаёт — значение по постановке шага 3 Э8).
//
// CSP — почему ровно такая (факты по репо, разведка шага 3):
//   • inline <script> и inline-обработчиков (on*/hx-on/javascript:) в
//     views/ НЕТ — все скрипты внешние, same-origin (/static/js/app.js,
//     /static/vendor/htmx.min.js, /static/vendor/qrcode-generator.js) →
//     script-src наследует 'self' от default-src; 'unsafe-inline' для
//     скриптов не нужен;
//   • htmx-атрибуты проекта (hx-get/hx-target/hx-swap/hx-indicator/
//     hx-push-url/hx-disabled-elt) eval не требуют → 'unsafe-eval'
//     НЕ ставится (постановка шага 3);
//   • inline style-атрибуты ЕСТЬ — ровно два (views/grades.csp:240 и
//     views/grades_discipline.csp:248, style="--value:N%" из серверных
//     констант) → style-src 'self' 'unsafe-inline': без него браузер
//     молча вырезает style-АТРИБУТЫ (прогресс-бары оценок ломаются),
//     при этом внешние стили остаются 'self' (tokens.css/app.css).
//
// Механика — ОДНА точка на процесс, не по-роутерно: advice на СОЗДАНИЕ
// ответа (registerHttpResponseCreationAdvice). По исходникам v1.9.14
// он вызывается во всех фабриках HttpResponseImpl.cc (newHttpResponse
// ×2 :70,:78; newHttpJsonResponse ×2 :86,:94; newRedirectionResponse
// :204; newFileResponse :266,:404,:466; newStreamResponse :482), значит
// покрывает контроллеры, ответы фильтров (401 JSON / 303 / 403 CSRF),
// статические файлы (StaticFileRouter ходит через newFileResponse:
// :425,:569,:585,:593) и кэшируемую 404-заглушку роутера (создаётся один
// раз через newHttpViewResponse → genHttpResponse → newHttpResponse,
// std::call_once — заголовки «запекаются» в кэш). Новый роут получает
// заголовки сам, без правок контроллеров (то же свойство «нет дрейфа»,
// что у CSRF-проверки из LoginFilter — Э8 шаг 2).
//
// ГВАРД «set-if-absent» — урок Э3 (двойной Content-Type у /ics: addHeader
// ДОБАВЛЯЕТ, не заменяет): при единственной регистрации advice заголовка
// ещё нет, гвард страхует от повторного вызова/регистрации и не плодит
// дубликаты.
//
// Не проходит через фабрики только 101-handshake /ws (собирается минуя
// HttpResponse-фабрики; заголовки для протокольного ответа без тела не
// применимы). Отказ /ws без cookie (401 от LoginFilter) — обычная фабрика,
// заголовки на месте; CSP на JSON/PDF/ICS безвреден.
class SecurityHeaders
{
  public:
    static constexpr const char* kContentTypeOptions = "X-Content-Type-Options";
    static constexpr const char* kContentTypeOptionsValue = "nosniff";
    static constexpr const char* kFrameOptions = "X-Frame-Options";
    static constexpr const char* kFrameOptionsValue = "DENY";
    static constexpr const char* kCsp = "Content-Security-Policy";
    static constexpr const char* kCspValue =
        "default-src 'self'; style-src 'self' 'unsafe-inline'; "
        "frame-ancestors 'none'";
    static constexpr const char* kReferrerPolicy = "Referrer-Policy";
    static constexpr const char* kReferrerPolicyValue =
        "strict-origin-when-cross-origin";

    // Идемпотентная расстановка четырёх заголовков (set-if-absent).
    static void apply(const drogon::HttpResponsePtr& resp);

    // Регистрация advice на создание ответа — ОДИН вызов на процесс:
    // main.cpp и security_tests (сервер теста конфигурационно идентичен
    // продовому).
    static void registerAdvice();
};

}  // namespace para
