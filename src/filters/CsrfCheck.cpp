#include "filters/CsrfCheck.h"

#include <drogon/HttpAppFramework.h>

#include <openssl/crypto.h>
#include <openssl/sha.h>

#include <string_view>
#include <utility>

#include "services/SessionService.h"
#include "utils/ApiError.h"

namespace para
{

namespace
{

std::string toHex(const unsigned char* data, std::size_t len)
{
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(len * 2);
    for (std::size_t i = 0; i < len; ++i)
    {
        out.push_back(hex[data[i] >> 4]);
        out.push_back(hex[data[i] & 0x0F]);
    }
    return out;
}

// Сравнение токенов — constant-time (CRYPTO_memcmp): ответ на неверный
// токен не должен различаться по времени в зависимости от совпавшего
// префикса (в духе Э1-находки о timing-оракуле входа — фиксируем в Э8).
bool constantTimeEquals(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;
    return CRYPTO_memcmp(a.data(), b.data(), a.size()) == 0;
}

}  // namespace

std::string CsrfCheck::tokenFor(const std::string& sessionToken)
{
    // Доменное разделение: префикс отделяет вывод от других хэшей тех же
    // входных данных, если появятся. Вход — 128-бит CSPRNG-токен сессии.
    const std::string input = "para-csrf-v1:" + sessionToken;
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.data()),
           input.size(),
           md);
    return toHex(md, SHA256_DIGEST_LENGTH);
}

drogon::Cookie CsrfCheck::cookieFor(const std::string& sessionToken,
                                    int64_t ttlSec)
{
    drogon::Cookie cookie(kCookieName, tokenFor(sessionToken));
    cookie.setPath("/");
    cookie.setMaxAge(static_cast<int>(ttlSec));
    cookie.setSecure(true);
    cookie.setSameSite(drogon::Cookie::SameSite::kLax);
    // HttpOnly НЕ ставится сознательно: значение читает app.js (перехватчик
    // добавляет X-CSRF-Token ко всем не-GET, 02 §6.4 double-submit).
    // ВАЖНО: в v1.9.14 httpOnly_ у Cookie дефолтен true (Cookie.h) — без
    // явного false тест ловил cookie с HttpOnly (поймано прогоном шага 2).
    cookie.setHttpOnly(false);
    return cookie;
}

std::optional<drogon::HttpResponsePtr> CsrfCheck::verify(
    const drogon::HttpRequestPtr& req)
{
    const std::string sessionToken =
        req->getCookie(SessionService::kCookieName);
    const std::string expected = tokenFor(sessionToken);
    // Зеркало токена: заголовок (htmx/fetch-путь app.js) или поле формы
    // (fold-up HTML-форм, 02 §6.4 «скрытое поле/заголовок»).
    std::string submitted = req->getHeader(kHeaderName);
    if (submitted.empty())
        submitted = req->getParameter(kFormField);
    const std::string inCookie = req->getCookie(kCookieName);
    // Три инварианта double-submit: submitted == cookie csrf (сам паттерн)
    // == tokenFor(session) (привязка к сессии — «signed double-submit»,
    // OWASP: подкинутая cookie втёмную не проходит).
    const bool ok = !submitted.empty() &&
                    constantTimeEquals(submitted, inCookie) &&
                    constantTimeEquals(inCookie, expected);
    if (ok)
        return std::nullopt;
    // Формат 02 §5.1; message без деталей механизма — пользователю хватает
    // «обновите страницу»: advice выдаст свежий токен на следующей странице.
    return api::errorResponse("csrf",
                              "Страница устарела — обновите её и повторите "
                              "действие",
                              drogon::k403Forbidden);
}

void CsrfCheck::registerPageIssuanceAdvice()
{
    drogon::app().registerPostHandlingAdvice(
        [](const drogon::HttpRequestPtr& req,
           const drogon::HttpResponsePtr& resp) {
            if (!resp || req->method() != drogon::Get)
                return;
            const std::string sessionToken =
                req->getCookie(SessionService::kCookieName);
            if (sessionToken.empty())
                return;
            // 1) Cookie csrf расходится с сессией (удалена вручную, осталась
            //    от прежней сессии) — выдаём актуальную: восстановление без
            //    повторного входа. Идемпотентно: верная cookie не трогается.
            const std::string expected = tokenFor(sessionToken);
            if (!constantTimeEquals(req->getCookie(kCookieName), expected))
                resp->addCookie(
                    cookieFor(sessionToken, SessionService::currentTtlSec()));
            // 2) <meta name="csrf-token"> в <head> страницы. Страницы
            //    (все view-шаблоны с partials/head.csp) имеют ровно один
            //    "</head>"; содержимое — экранированные серверные данные
            //    (02 §6.4), сырые "</head>" в теле невозможны. Инъекция
            //    одноразовая (мета уже есть — не добавляем). Фрагменты htmx
            //    (schedule-партиалы, staff/app-card) "</head>" не содержат —
            //    мета им не нужна: htmx-запросы проекта — только GET.
            const std::string_view bodyView = resp->body();
            std::string body(bodyView);
            const std::size_t headEnd = body.find("</head>");
            if (headEnd == std::string::npos ||
                body.find("name=\"csrf-token\"") != std::string::npos)
                return;
            body.insert(headEnd,
                        std::string("<meta name=\"csrf-token\" content=\"") +
                            expected + "\">\n");
            resp->setBody(std::move(body));
        });
}

}  // namespace para
