// Мост словаря 5.18 в рендер страниц — реализации (контракт: utils/I18nView.h).

#include "utils/I18nView.h"

#include <string>
#include <string_view>

namespace para::i18n
{
namespace
{

// Таблица языка (kRu — источник истины keyset; en/zh покрывают его целиком —
// static_assert в models/i18n.h, фолбэк t() здесь не нужен).
const Entry* tableFor(Lang lang)
{
    switch (lang)
    {
    case Lang::En: return kEn;
    case Lang::Zh: return kZh;
    case Lang::Ru: break;
    }
    return kRu;
}

// JSON-строка для <script type="application/json">: значения — серверные
// constexpr-константы словаря (кавычек/обратных слэшей/угловых скобок в них
// нет), но экранируем по правилам JSON+script-контекста всё равно —
// защищает от будущих правок словаря. Управляющие символы в словаре
// отсутствуют (проверено noEmptyValues + инвентарь шага 1), последователь
// ность «</script» тоже — угловая скобка экранируется \u003c на всякий.
std::string jsonEscape(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (const char c : s)
    {
        switch (c)
        {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '<': out += "\\u003c"; break;
        case '>': out += "\\u003e"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out += c; break;
        }
    }
    return out;
}

}  // namespace

Lang resolveRequestLang(const drogon::HttpRequestPtr& req)
{
    return resolveLang(req->getCookie("para_lang"),
                       req->getHeader("Accept-Language"));
}

void insertUiStrings(drogon::HttpViewData& data, Lang lang)
{
    data.insert("lang", std::string(langCode(lang)));
    const Entry* const table = tableFor(lang);
    const std::size_t count = sizeof(kRu) / sizeof(kRu[0]);
    static_assert(sizeof(kEn) / sizeof(kEn[0]) == count &&
                      sizeof(kZh) / sizeof(kZh[0]) == count,
                  "i18n tables must stay the same size (sameKeyset)");
    std::string json;
    json.reserve(count * 60);
    json += "{";
    for (std::size_t i = 0; i < count; ++i)
    {
        data.insert("t." + std::string(table[i].key),
                    std::string(table[i].value));
        if (i != 0) json += ",";
        json += "\"";
        json += table[i].key;
        json += "\":\"";
        json += jsonEscape(table[i].value);
        json += "\"";
    }
    json += "}";
    data.insert("i18nJson", std::move(json));
}

std::string pageTitleOf(std::string_view screenKey, Lang lang)
{
    std::string out(t(screenKey, lang));
    out += " · ";
    out += t("app.title", lang);
    return out;
}

}  // namespace para::i18n
