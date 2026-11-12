#include "controllers/LangCtrl.h"

#include <string>

#include "models/i18n.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void LangCtrl::set(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Валидация кода — единый список проекта (i18n.h: ru|en|zh, те же
    // значения, что CHECK миграции 0008 и settings.lang); иное — 400 в
    // формате 02 §5.1 (кнопки переключателя шлют только допустимые, 400 —
    // крайний случай ручной ссылки/кэша).
    const std::string lang = req->getParameter("lang");
    if (!para::i18n::langFromCode(lang).has_value())
    {
        cb(api::errorResponse("bad_request",
                              "lang — допустимо ru, en или zh",
                              k400BadRequest));
        return;
    }

    // Cookie para_lang — источник истины резолва языка рендером (шаг 3:
    // para_lang → Accept-Language → ru). Флаги — по прецеденту csrf-cookie
    // (CsrfCheck::cookieFor, Э8): Secure; SameSite=Lax; БЕЗ HttpOnly —
    // значение читает app.js (подсветка активной кнопки сегмента). ВАЖНО
    // (урок Э8 шаг 2): httpOnly_ у drogon::Cookie дефолтно true — снимаем
    // явно. Max-Age — год: выбор языка переживает закрытие браузера и
    // перелогин (сессия коротка, язык — предпочтение устройства).
    Cookie cookie("para_lang", lang);
    cookie.setPath("/");
    cookie.setMaxAge(31536000);  // 365 дней
    cookie.setSecure(true);
    cookie.setSameSite(Cookie::SameSite::kLax);
    cookie.setHttpOnly(false);

    // Возврат на прежнюю страницу: next — только ЛОКАЛЬНЫЙ путь (анти
    // open redirect): непустой, ведёт «/…», не прото-относительный «//…»,
    // без «\», CR/LF (заголовок Location). Иное — на корень.
    std::string next = req->getParameter("next");
    const bool localNext =
        !next.empty() && next.front() == '/' &&
        next.rfind("//", 0) != 0 && next.find('\\') == std::string::npos &&
        next.find('\r') == std::string::npos &&
        next.find('\n') == std::string::npos;
    if (!localNext)
        next = "/";

    auto resp = HttpResponse::newHttpResponse();
    resp->addCookie(std::move(cookie));
    resp->setStatusCode(k302Found);
    resp->addHeader("Location", next);
    cb(resp);
}
