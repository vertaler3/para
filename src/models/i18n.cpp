// Резолв языка запроса — фича 5.18, шаг 1 (декларации — models/i18n.h,
// там же словарь ru/en/zh и t(key, lang)). Контракт — в комментарии шапки
// заголовка; юнит-тесты t()/резолва — шаг 7 (постановка фичи).

#include "models/i18n.h"

namespace para::i18n
{
namespace
{

// ASCII-пробелы (заголовки содержат только их; не locale-зависимо).
constexpr bool isSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::string_view trim(std::string_view s)
{
    while (!s.empty() && isSpace(s.front()))
        s.remove_prefix(1);
    while (!s.empty() && isSpace(s.back()))
        s.remove_suffix(1);
    return s;
}

constexpr char lowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

// q-значение: «1» / «0.8» / «.5» → double. Мусор («0,8», «abc», пусто) →
// nullopt — вызывающий трактует как «q не задан» (вес 1, RFC 7231 §5.3.1:
// «If … qvalue is not present … default weight of 1»). Ручной разбор без
// strtod — без locale и аллокаций.
std::optional<double> parseQ(std::string_view s)
{
    s = trim(s);
    if (s.empty())
        return std::nullopt;
    double value = 0.0;
    std::size_t i = 0;
    bool digits = false;
    while (i < s.size() && s[i] >= '0' && s[i] <= '9')
    {
        value = value * 10.0 + static_cast<double>(s[i] - '0');
        ++i;
        digits = true;
    }
    if (i < s.size() && s[i] == '.')
    {
        ++i;
        double scale = 0.1;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9')
        {
            value += static_cast<double>(s[i] - '0') * scale;
            scale /= 10.0;
            ++i;
            digits = true;
        }
    }
    if (!digits || i != s.size())
        return std::nullopt;
    return value;
}

// Основной подтег диапазона: «zh-CN» → «zh», «EN」 → 「en」 (ASCII-нижний
// регистр — теги языка ASCII). Пишет в buf (вместимость cap, включая место
// под буфер ≤ 8 символов тега + защита от переполнения); пусто — если
// подтег длиннее cap-1 или диапазон пуст. Подтег языка длиннее 8 символов
// в BCP 47 не встречается — это мусор, не язык.
std::string_view primaryTag(std::string_view range, char* buf,
                            std::size_t cap)
{
    std::size_t n = 0;
    for (const char c : range)
    {
        if (c == '-' || c == '_')
            break;
        if (n + 1 >= cap)
            return {};
        buf[n++] = lowerAscii(c);
    }
    if (n == 0)
        return {};
    return {buf, n};
}

}  // namespace

std::optional<Lang> langFromCookie(std::string_view cookieValue)
{
    return langFromCode(trim(cookieValue));
}

std::optional<Lang> langFromAcceptLanguage(std::string_view header)
{
    std::optional<Lang> best;
    double bestQ = 0.0;  // планка: q<=0 («не приемлем») не выбирается
    std::size_t begin = 0;
    while (begin <= header.size())
    {
        std::size_t end = header.find(',', begin);
        if (end == std::string_view::npos)
            end = header.size();
        const std::string_view item = header.substr(begin, end - begin);
        begin = end + 1;

        // «диапазон; параметры» — параметры после первого ';' (q может
        // быть не единственным параметром — ищем сегмент с именем q).
        std::string_view range = item;
        std::string_view params;
        const std::size_t semicolon = item.find(';');
        if (semicolon != std::string_view::npos)
        {
            range = item.substr(0, semicolon);
            params = item.substr(semicolon + 1);
        }
        range = trim(range);
        if (range.empty() || range == "*")
            continue;  // «*» язык не называет — на него не резолвим

        double q = 1.0;  // без q — максимальный вес
        std::size_t ppos = 0;
        while (ppos <= params.size())
        {
            std::size_t pend = params.find(';', ppos);
            if (pend == std::string_view::npos)
                pend = params.size();
            const std::string_view param =
                trim(params.substr(ppos, pend - ppos));
            ppos = pend + 1;
            const std::size_t eq = param.find('=');
            if (eq == std::string_view::npos)
                continue;
            const std::string_view name = trim(param.substr(0, eq));
            if (name.size() == 1 && lowerAscii(name[0]) == 'q')
                if (const auto parsed = parseQ(param.substr(eq + 1)))
                    q = *parsed;
        }
        if (q <= 0.0)
            continue;

        char buf[9];
        if (const auto lang = langFromCode(primaryTag(range, buf, sizeof buf)))
        {
            if (!best || q > bestQ)  // строго больше: при равных — первый
            {
                best = lang;
                bestQ = q;
            }
        }
    }
    return best;
}

Lang resolveLang(std::string_view cookieValue,
                 std::string_view acceptLanguage)
{
    if (const auto fromCookie = langFromCookie(cookieValue))
        return *fromCookie;
    if (const auto fromHeader = langFromAcceptLanguage(acceptLanguage))
        return *fromHeader;
    return defaultLang();
}

}  // namespace para::i18n
