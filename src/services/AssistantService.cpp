#include "services/AssistantService.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <string_view>
#include <vector>

#include <drogon/HttpClient.h>
#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "services/DashboardService.h"
#include "services/GradesService.h"
#include "services/NewsService.h"
#include "services/ScheduleService.h"

using namespace drogon;
using drogon::orm::DrogonDbException;

namespace para {

namespace {

std::string envOr(const char* name)
{
    const char* v = std::getenv(name);
    return v == nullptr ? std::string() : std::string(v);
}

// Время «сейчас» в формате проекта (ISO-8601 UTC «…Z»): сравнение строк
// лексикографическое — как в DashboardService/ReminderService.
std::string nowIso()
{
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

// «…Z» → time_t (UTC); -1 — не разобрали.
std::time_t parseIso(const std::string& iso)
{
    std::tm tm{};
    if (iso.size() < 16 ||
        std::sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d", &tm.tm_year,
                    &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min) != 5)
        return -1;
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return timegm(&tm);
}

// Форматирование времени контекста БЕЗ перевода поясов — как в остальных
// экранах портала: чат и расписание называют одно и то же время (фикс
// 21.11: раньше контекст сдвигал UTC→МСК и модель говорила «13:00» там,
// где UI показывает «10:00»).
std::string fmtPortal(const std::string& iso, const char* pattern)
{
    const std::time_t t = parseIso(iso);
    if (t < 0)
        return iso;
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof buf, pattern, &tm);
    return buf;
}

// «Сегодня» в UTC-календаре проекта — тот же «сегодня», что в
// /v1/schedule/today и на дашборде.
std::string todayUtc()
{
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &tm);
    return buf;
}

// День ISO-строки «…Z» как есть (UTC-календарь портала).
std::string isoDay(const std::string& iso)
{
    return iso.size() >= 10 ? iso.substr(0, 10) : iso;
}

const char* weekdayRu(const std::string& date)
{
    static const char* kNames[] = {"воскресенье", "понедельник", "вторник",
                                   "среда",       "четверг",     "пятница",
                                   "суббота"};
    const std::time_t t = parseIso(date + "T12:00");
    if (t < 0)
        return "";
    std::tm tm{};
    gmtime_r(&t, &tm);
    return kNames[tm.tm_wday];
}

std::string lessonKind(const std::string& k)
{
    if (k == "lecture")
        return "лекция";
    if (k == "seminar")
        return "семинар";
    return k;
}

std::string planKind(const std::string& k)
{
    if (k == "consultation")
        return "консультация";
    if (k == "exam")
        return "экзамен";
    if (k == "credit")
        return "зачёт";
    if (k == "retake")
        return "пересдача";
    if (k == "work")
        return "работа";
    return k;
}

std::string str(const Json::Value& v, const char* key)
{
    const auto& x = v[key];
    return x.isString() ? x.asString() : std::string();
}

void renderLesson(std::string& out, const Json::Value& e)
{
    out += "- " + AssistantService::portalTime(str(e, "starts_at")) + "–" +
           AssistantService::portalTime(str(e, "ends_at")) + " " +
           str(e, "discipline") + " (" + lessonKind(str(e, "kind")) +
           "), ауд. " + str(e, "room");
    if (!str(e, "building").empty())
        out += ", " + str(e, "building");
    const std::string status = str(e, "status");
    if (status == "cancelled")
        out += " — ОТМЕНЕНА";
    else if (status == "moved")
    {
        out += " — ПЕРЕНЕСЕНА";
        std::string was;
        if (!str(e, "prev_starts_at").empty())
            was += AssistantService::portalDateTime(str(e, "prev_starts_at"));
        if (!str(e, "prev_room").empty())
            was += (was.empty() ? "ауд. " : ", ауд. ") + str(e, "prev_room");
        if (!was.empty())
            out += " (было: " + was + ")";
    }
    else if (status == "substituted")
        out += " — замена преподавателя";
    out += "\n";
}

}  // namespace

// --- Настройки ---------------------------------------------------------------

namespace {

// Канонический системный промт ассистента (фикс 21.11, журнал
// 2026-11-21-ассистент-фиксы.md). Живёт В КОДЕ, а не только в .env:
// .env в gitignore (§5.2), и git-деплой не переносил промт на сервер —
// там модель работала со старым «мягким» текстом и снова подтверждала
// решения («ваш расчёт верен»). Env-переменная ASSISTANT_SYSTEM_PROMPT
// остаётся ПЕРЕОПРЕДЕЛЕНИЕМ для особых окружений.
const char* kDefaultSystemPrompt = R"(Ты — «Ассистент Пара», помощник студенческого портала «Пара» (концепт хакатона, данные вымышленные). Отвечай ТОЛЬКО по данным из контекста запроса; если нужных данных в контексте нет — скажи «в портале нет такой информации» и укажи реальный раздел портала, где это можно посмотреть.
НИКОГДА не подставляй данные пользователя вместо данных о других группах/событиях: вопрос о чужой группе/аудитории/событии, которого нет в контексте, — это «нет информации», а не расписание пользователя.
ПРОВЕРКА РЕШЕНИЙ: не проверяй и не переписывай решение целиком — только намёки («проверь единицы измерения во втором шаге»), наводящие вопросы и похожие примеры другой формулировки. Никогда не говори «ваш расчёт верен»; если видишь вероятную ошибку — укажи, на какой шаг посмотреть, без готового итога. Считай аккуратно; если сомневаешься в точности — так и скажи.
ВРЕМЯ: пиши время ровно так, как оно записано в контексте (как в остальном интерфейсе портала), БЕЗ перевода часовых поясов и без сдвигов.
НАВИГАЦИЯ: называй только реально существующие разделы портала: «Сегодня» (дашборд), «Расписание» (+ сегмент «Неделя», ссылка «Забронировать аудиторию»), «Успеваемость», «Заявки» (+ «Новая заявка»), «Профиль», «Библиотека», «Столовая», «Карта», «Люди», «Зачётка», «Рейтинг», «Стол сотрудника» (только для сотрудников). Никогда не отправляй в разделы «Помощь» или «к администрации» — их нет в портале.
Кратко (1–5 предложений), дружелюбно, язык = языку вопроса; персональные данные других студентов не раскрывай; без markdown-таблиц.)";

}  // namespace

std::string AssistantService::unquoteEnv(const std::string& raw)
{
    if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'')
        return raw.substr(1, raw.size() - 2);
    if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"')
        return raw;
    std::string out;
    out.reserve(raw.size());
    for (std::size_t i = 1; i + 1 < raw.size(); ++i)
    {
        const char c = raw[i];
        if (c == '\\' && i + 2 < raw.size())
        {
            const char n = raw[++i];
            switch (n)
            {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                default: out += '\\'; out += n; break;
            }
            continue;
        }
        out += c;
    }
    return out;
}

std::string AssistantService::resolveModel(const std::string& model,
                                           const std::string& folderId)
{
    if (model.empty() || model.find("://") != std::string::npos ||
        folderId.empty())
        return model;
    // «yandexgpt-lite» → gpt://<каталог>/yandexgpt-lite/latest;
    // «yandexgpt-lite/rc» (версия указана) — без добавки /latest.
    const bool hasVersion = model.find('/') != std::string::npos;
    return "gpt://" + folderId + "/" + model + (hasVersion ? "" : "/latest");
}

AssistantService::Config AssistantService::configFromEnv()
{
    Config c;
    c.url = unquoteEnv(envOr("ASSISTANT_API_URL"));
    c.key = unquoteEnv(envOr("ASSISTANT_API_KEY"));
    c.model = resolveModel(unquoteEnv(envOr("ASSISTANT_MODEL")),
                           unquoteEnv(envOr("ASSISTANT_FOLDER_ID")));
    // Промт: env-значение ПЕРЕОПРЕДЕЛЯЕТ встроенный канонический текст
    // (kDefaultSystemPrompt). Пустое/отсутствующее значение — не «не
    // настроено», а «использовать дефолт»: иначе git-деплой без ручного
    // переноса .env тихо оставлял сервер на старом промте (инцидент
    // 21.11 — «ассистент снова решает задания»).
    c.systemPrompt = unquoteEnv(envOr("ASSISTANT_SYSTEM_PROMPT"));
    if (c.systemPrompt.empty())
        c.systemPrompt = kDefaultSystemPrompt;
    return c;
}

// --- Валидация -----------------------------------------------------------------

std::size_t AssistantService::utf8Length(const std::string& s)
{
    std::size_t n = 0;
    for (const unsigned char c : s)
        if ((c & 0xC0) != 0x80)  // не байт-продолжение
            ++n;
    return n;
}

AssistantService::Input AssistantService::validate(const Json::Value* body,
                                                   std::string& out)
{
    out.clear();
    if (body == nullptr || !body->isObject() || !(*body)["message"].isString())
        return Input::Empty;
    const std::string raw = (*body)["message"].asString();
    const auto first = raw.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return Input::Empty;
    const auto last = raw.find_last_not_of(" \t\r\n");
    out = raw.substr(first, last - first + 1);
    if (utf8Length(out) > kMaxMessageChars)
    {
        out.clear();
        return Input::TooLong;
    }
    return Input::Ok;
}

// --- Лимитер -----------------------------------------------------------------

int64_t AssistantService::RateLimiter::acquire(int64_t userId, int64_t nowSec)
{
    std::lock_guard<std::mutex> lock(mu_);
    auto& q = hits_[userId];
    while (!q.empty() && q.front() <= nowSec - kWindowSec)
        q.pop_front();
    if (static_cast<int>(q.size()) >= kMax)
        return std::max<int64_t>(1, q.front() + kWindowSec - nowSec);
    q.push_back(nowSec);
    return 0;
}

AssistantService::RateLimiter& AssistantService::RateLimiter::instance()
{
    static RateLimiter limiter;
    return limiter;
}

// --- Контекст ------------------------------------------------------------------

std::string AssistantService::portalTime(const std::string& iso)
{
    return fmtPortal(iso, "%H:%M");
}

std::string AssistantService::portalDateTime(const std::string& iso)
{
    return fmtPortal(iso, "%d.%m %H:%M");
}

// Верхний регистр для ASCII + кириллицы (UTF-8, U+0430..U+044F, Ё U+0451).
std::string upperRu(const std::string& s)
{
    std::string upper;
    for (std::size_t k = 0; k < s.size(); ++k)
    {
        unsigned char a = s[k],
                      b = k + 1 < s.size() ? s[k + 1] : '\0';
        if (a == 0xD0 && b >= 0xB0 && b <= 0xBF)  // а..п → А..П
        {
            upper += a;
            upper += static_cast<char>(b - 0x20);
            ++k;
        }
        else if (a == 0xD1 && b == 0x91)  // ё → Ё
        {
            upper += a;
            upper += static_cast<char>(0x81);
            ++k;
        }
        else if (a == 0xD1 && b >= 0x80 && b <= 0x8F)  // р..я → Р..Я
        {
            upper += static_cast<char>(0xD0);
            upper += static_cast<char>(b + 0x20);
            ++k;
        }
        else
            upper += static_cast<char>(
                std::toupper(static_cast<unsigned char>(a)));
    }
    return upper;
}

std::string AssistantService::roomQuery(const std::string& message)
{
    // Токен = буквы/цифры/дефис; валиден, если 0–2 БУКВЫ + необязательный
    // дефис + 3–4 ЦИФРЫ («1207», «3105», «Б-204», «в-216» → «В-216»).
    // Годы и проценты не фильтруем специально: LIKE по rooms честно
    // вернёт пусто — в контекст ничего лишнего не попадёт.
    std::string token;
    auto flush = [&token]() {
        if (token.empty())
            return std::string();
        std::string letters, digits;
        std::size_t i = 0;
        for (; i < token.size() &&
               (std::isalpha(static_cast<unsigned char>(token[i])) ||
                token[i] & 0x80);  // кириллица — байты >= 0x80
             ++i)
            letters += token[i];
        if (i < token.size() && token[i] == '-')
            ++i;
        digits = token.substr(i);
        const bool shape = digits.size() >= 3 && digits.size() <= 4 &&
                           digits.find_first_not_of("0123456789") ==
                               std::string::npos &&
                           letters.size() <= 4;  // ≤2 буквы в UTF-8 = ≤4 байт
        if (!shape)
        {
            token.clear();
            return std::string();
        }
        std::string result = upperRu(letters);
        if (!letters.empty() && i > 0 && token[i - 1] == '-')
            result += "-";
        result += digits;
        token.clear();
        return result;
    };
    for (const char c : message)
    {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' ||
            static_cast<unsigned char>(c) >= 0x80)  // кириллица
            token += c;
        else
        {
            const std::string found = flush();
            if (!found.empty())
                return found;
        }
    }
    return flush();
}

std::string AssistantService::mentionedGroup(const std::string& message)
{
    // Токены букв/цифр/дефиса; валиден, если 1–2 ЦИФРЫ + дефис + 2–6 БУКВ
    // («4-МЭО», «1-фмо» → «1-ФМО»). Даты и номера аудиторий не подходят.
    std::string token;
    auto flush = [&token]() {
        if (token.empty())
            return std::string();
        std::size_t i = 0;
        while (i < token.size() &&
               std::isdigit(static_cast<unsigned char>(token[i])))
            ++i;
        const std::string digits = token.substr(0, i);
        const bool shape = digits.size() >= 1 && digits.size() <= 2 &&
                           i < token.size() && token[i] == '-';
        if (!shape)
        {
            token.clear();
            return std::string();
        }
        ++i;  // дефис
        std::string letters = token.substr(i);
        token.clear();
        if (letters.empty() || letters.size() > 12)  // ≤6 букв в UTF-8
            return std::string();
        for (const char c : letters)
        {
            const unsigned char b = static_cast<unsigned char>(c);
            // буквы: ASCII-буквы или байты кириллицы (>= 0x80); цифры и
            // прочее — не группа.
            if (b < 0x80 && !std::isalpha(b))
                return std::string();
        }
        return digits + "-" + upperRu(letters);
    };
    for (const char c : message)
    {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' ||
            static_cast<unsigned char>(c) >= 0x80)
            token += c;
        else
        {
            const std::string found = flush();
            if (!found.empty())
                return found;
        }
    }
    return flush();
}

namespace {

// «2026-11-25» + дней → «2026-12-02» (арифметика через timegm/gmtime).
std::string addDaysStr(const std::string& ymd, int days)
{
    const std::time_t t = parseIso(ymd + "T12:00:00Z");
    if (t < 0)
        return std::string();
    std::tm tm{};
    const std::time_t shifted = t + days * 86400;
    gmtime_r(&shifted, &tm);
    char buf[16];
    std::strftime(buf, sizeof buf, "%Y-%m-%d", &tm);
    return buf;
}

int dayOfMonth(const std::string& ymd)
{
    return ymd.size() >= 10 ? std::atoi(ymd.c_str() + 8) : 0;
}

}  // namespace

std::string AssistantService::mentionedDate(const std::string& message,
                                            const std::string& today)
{
    if (today.size() < 10)
        return std::string();
    const int todayYear = std::atoi(today.c_str());
    // Токены: буквы/цифры/точки (кириллица — байты >= 0x80).
    std::vector<std::string> tokens;
    std::string token;
    auto flush = [&tokens, &token]() {
        if (!token.empty())
        {
            tokens.push_back(token);
            token.clear();
        }
    };
    for (const char c : message)
    {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' ||
            static_cast<unsigned char>(c) >= 0x80)
            token += c;
        else
            flush();
    }
    flush();
    if (tokens.empty())
        return std::string();

    // Месяцы по префиксу в ВЕРХНЕМ регистре (токен прогоняется через
    // upperRu; именительный/родительный: «ноябрь»/«ноября»).
    static const std::pair<const char*, int> kMonths[] = {
        {"ЯНВ", 1},  {"ФЕВ", 2},  {"МАР", 3},  {"АПР", 4},  {"МАЯ", 5},
        {"МАЙ", 5},  {"МАЕ", 5},  {"МАЮ", 5},  {"ИЮН", 6},  {"ИЮЛ", 7},
        {"АВГ", 8},  {"СЕН", 9},  {"ОКТ", 10}, {"НОЯ", 11}, {"ДЕК", 12}};
    // Будни (в любом падеже — матчится по префиксу), верхний регистр.
    static const std::pair<const char*, int> kWeekdays[] = {
        {"ПОНЕДЕЛ", 1}, {"ВТОРНИ", 2},  {"СРЕД", 3}, {"ЧЕТВЕР", 4},
        {"ПЯТНИЦ", 5},   {"СУББОТ", 6},  {"ВОСКРЕСЕН", 0}};

    const auto toUpper = [](const std::string& s) { return upperRu(s); };

    for (std::size_t i = 0; i < tokens.size(); ++i)
    {
        const std::string& t = tokens[i];
        // 1) «25.11» / «25.11.2026» — целиком цифро-точечный токен.
        // Токен без точки («25») сюда не попадает и уходит в правило 2
        // (число + месяц).
        if (t.find_first_not_of("0123456789.") == std::string::npos &&
            t.find('.') != std::string::npos)
        {
            int d = 0, m = 0, y = 0;
            const auto first = t.find('.');
            const auto second = t.find('.', first + 1);
            d = std::atoi(t.substr(0, first).c_str());
            if (second == std::string::npos)
                m = std::atoi(t.substr(first + 1).c_str());
            else
            {
                m = std::atoi(t.substr(first + 1, second - first - 1)
                                  .c_str());
                y = std::atoi(t.substr(second + 1).c_str());
            }
            if (d >= 1 && d <= 31 && m >= 1 && m <= 12 &&
                (y == 0 || (y >= 2000 && y <= 2100)))
            {
                int year = y != 0 ? y : todayYear;
                char buf[16];
                std::snprintf(buf, sizeof buf, "%04d-%02d-%02d", year, m,
                              d);
                std::string ymd(buf);
                if (ymd < today && y == 0)
                    ymd = addDaysStr(ymd, 365);  // спрашивают будущее
                return ymd;
            }
            continue;
        }
        // 2) «25 ноября» — число + следующий токен месяц.
        if (t.find_first_not_of("0123456789") == std::string::npos)
        {
            const int d = std::atoi(t.c_str());
            if (d >= 1 && d <= 31 && i + 1 < tokens.size())
            {
                const std::string next = toUpper(tokens[i + 1]);
                for (const auto& [p, m] : kMonths)
                    if (next.rfind(p, 0) == 0)
                    {
                        char buf[16];
                        std::snprintf(buf, sizeof buf, "%04d-%02d-%02d",
                                      todayYear, m, d);
                        std::string ymd(buf);
                        if (ymd < today)
                            ymd = addDaysStr(ymd, 365);
                        return ymd;
                    }
            }
            continue;
        }
        // 3) «в среду» / «среда» — ближайший будень СТРОГО после today.
        {
            const std::string up = toUpper(t);
            for (const auto& [p, wd] : kWeekdays)
                if (up.rfind(p, 0) == 0)
                {
                    const std::time_t cur =
                        parseIso(today + "T12:00:00Z");
                    if (cur < 0)
                        return std::string();
                    std::tm tm{};
                    gmtime_r(&cur, &tm);
                    int delta = (wd - tm.tm_wday + 7) % 7;
                    if (delta == 0)
                        delta = 7;
                    return addDaysStr(today, delta);
                }
        }
    }
    return std::string();
}

std::string AssistantService::renderContext(const ContextData& d)
{
    const std::string tomorrow = ScheduleService::addDays(d.today, 1);
    std::string out;
    out += "ДАННЫЕ ПОРТАЛА ДЛЯ ЭТОГО СТУДЕНТА (вымышленные; время — как в "
           "интерфейсе портала, без перевода поясов):\n";
    out += "Сегодня " + d.today + ", " + weekdayRu(d.today) + ".\n";
    out += "Ниже — только твои данные: расписание твоей группы и твои дела."
           " Данных о других группах в этом контексте нет.\n";
    if (!d.group.empty())
        out += "Твоя группа — " + d.group + ".\n";
    if (!d.foreignGroup.empty())
        out += "ВНИМАНИЕ: в вопросе упомянута группа " + d.foreignGroup +
               " — это НЕ твоя группа, и данных о ней в этом контексте"
               " нет. На вопрос о группе " + d.foreignGroup +
               " отвечай: «в портале нет такой информации» (расписание"
               " другой группы смотри в разделе «Расписание» на месте)."
               " Своё расписание вместо группы " + d.foreignGroup +
               " НЕ подставляй.\n";
    if (d.solutionCheck)
        out += "ВНИМАНИЕ: студент просит проверить его решение. НЕ"
               " подтверждай и не опровергай итог, НЕ переписывай решение."
               " Задай ОДИН наводящий вопрос и укажи, какой шаг проверить"
               " (например, точную формулу или единицы измерения), без"
               " готового результата.\n";

    for (const auto& [day, label] :
         {std::pair{d.today, "Расписание твоей группы на сегодня"},
          std::pair{tomorrow, "Расписание твоей группы на завтра"}})
    {
        out += std::string("\n") + label + " (" + day + ", " + weekdayRu(day) +
               "):\n";
        bool any = false;
        if (d.events.isArray())
            for (const auto& e : d.events)
                if (isoDay(str(e, "starts_at")) == day)
                {
                    renderLesson(out, e);
                    any = true;
                }
        if (!any)
            out += "- пар нет\n";
    }

    // Дата из вопроса («что за пары 25 ноября») — догруженная секция.
    if (!d.dateAsked.empty())
    {
        static const char* kMonthsGen[] = {
            "января", "февраля", "марта",     "апреля",   "мая",
            "июня",   "июля",     "августа",  "сентября", "октября",
            "ноября", "декабря"};
        const int m = d.dateAsked.size() >= 10
                          ? std::atoi(d.dateAsked.c_str() + 5)
                          : 0;
        const int day = dayOfMonth(d.dateAsked);
        const int year = std::atoi(d.dateAsked.c_str());
        std::string title = d.dateAsked;
        if (m >= 1 && m <= 12 && day > 0)
            title = std::to_string(day) + " " + kMonthsGen[m - 1] + " " +
                    std::to_string(year);
        out += "\nРасписание твоей группы на " + title + ", " +
               weekdayRu(d.dateAsked) + ":\n";
        bool anyDate = false;
        if (d.dateEvents.isArray())
            for (const auto& e : d.dateEvents)
            {
                renderLesson(out, e);
                anyDate = true;
            }
        if (!anyDate)
            out += "- пар нет\n";
    }

    out += "\nБлижайшие консультации, экзамены и зачёты:\n";
    bool anyExam = false;
    const std::string now = nowIso();
    if (d.exams.isArray())
        for (const auto& e : d.exams)
            if (str(e, "starts_at") >= now)
            {
                out += "- " + portalDateTime(str(e, "starts_at")) + " " +
                       planKind(str(e, "kind")) + ": " + str(e, "discipline") +
                       ", ауд. " + str(e, "room") + "\n";
                anyExam = true;
            }
    if (!anyExam)
        out += "- нет\n";

    out += "\nАктивные дедлайны:\n";
    if (d.deadlines.isArray() && !d.deadlines.empty())
        for (const auto& x : d.deadlines)
            out += "- до " + portalDateTime(str(x, "due_at")) + ": " +
                   str(x, "title") + " (" + planKind(str(x, "kind")) + ")\n";
    else
        out += "- нет\n";

    out += "\nЗаявки в работе:\n";
    if (d.applications.isArray() && !d.applications.empty())
        for (const auto& a : d.applications)
            out += "- №" + std::to_string(a["id"].asInt64()) + " " +
                   str(a, "type_title") + " — статус: " +
                   std::string(models::applicationStatusTitle(
                       str(a, "status"))) +
                   "\n";
    else
        out += "- нет\n";

    out += "\nБлижайшие события кампуса:\n";
    if (d.campus.isArray() && !d.campus.empty())
        for (const auto& n : d.campus)
        {
            out += "- " + portalDateTime(str(n, "event_at")) + " " +
                   str(n, "title");
            if (!str(n, "location").empty())
                out += ", " + str(n, "location");
            out += "\n";
        }
    else
        out += "- нет\n";

    // Аудитории из вопроса (карта портала): раздел добавляем только при
    // непустом результате — пустой не подсказывает модели «нет аудитории».
    if (d.rooms.isArray() && !d.rooms.empty())
    {
        out += "\nАудитории по твоему вопросу (из карты портала):\n";
        for (const auto& r : d.rooms)
        {
            out += "- " + str(r, "name") + " — " + str(r, "building");
            if (r["floor"].isIntegral())
                out += ", этаж " + std::to_string(r["floor"].asInt64());
            if (!str(r, "kind").empty())
                out += " (" + str(r, "kind") + ")";
            out += "\n";
        }
    }
    return out;
}

Json::Value AssistantService::buildRequest(const Config& cfg,
                                           const std::string& contextText,
                                           const Json::Value& history,
                                           const std::string& message)
{
    Json::Value req(Json::objectValue);
    req["model"] = cfg.model;
    req["max_tokens"] = kMaxTokens;
    req["temperature"] = 0.3;
    Json::Value msgs(Json::arrayValue);
    Json::Value sys(Json::objectValue);
    sys["role"] = "system";
    sys["content"] = cfg.systemPrompt + "\n\n" + contextText;
    msgs.append(std::move(sys));
    if (history.isArray())
        for (const auto& h : history)
        {
            Json::Value m(Json::objectValue);
            m["role"] = h["role"].asString();
            m["content"] = h["content"].asString();
            msgs.append(std::move(m));
        }
    Json::Value user(Json::objectValue);
    user["role"] = "user";
    user["content"] = message;
    msgs.append(std::move(user));
    req["messages"] = std::move(msgs);
    return req;
}

std::optional<std::string> AssistantService::parseReply(int httpStatus,
                                                        const std::string& body)
{
    if (httpStatus != 200)
        return std::nullopt;
    Json::Value root;
    Json::CharReaderBuilder b;
    std::string errs;
    std::unique_ptr<Json::CharReader> reader(b.newCharReader());
    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs) ||
        !root.isObject())
        return std::nullopt;
    const auto& choices = root["choices"];
    if (!choices.isArray() || choices.empty())
        return std::nullopt;
    const auto& content = choices[0]["message"]["content"];
    if (!content.isString())
        return std::nullopt;
    std::string text = content.asString();
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return std::nullopt;
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// --- Транспорт -------------------------------------------------------------------

AssistantService::Transport AssistantService::httpTransport(
    trantor::EventLoop* loop)
{
    return [loop](const Config& cfg, const Json::Value& payload, ReplyCb cb) {
        // URL → origin (scheme://host[:port]) + path.
        const auto scheme = cfg.url.find("://");
        if (scheme == std::string::npos)
        {
            LOG_WARN << "ассистент: ASSISTANT_API_URL без схемы";
            cb(std::nullopt);
            return;
        }
        const auto slash = cfg.url.find('/', scheme + 3);
        const std::string origin =
            slash == std::string::npos ? cfg.url : cfg.url.substr(0, slash);
        const std::string path =
            slash == std::string::npos ? "/" : cfg.url.substr(slash);

        auto client = HttpClient::newHttpClient(origin, loop);
        auto req = HttpRequest::newHttpRequest();
        req->setMethod(Post);
        req->setPath(path);
        // Схема Api-Key (не Bearer) — YandexGPT API-ключ сервисного аккаунта.
        req->addHeader("Authorization", "Api-Key " + cfg.key);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        Json::StreamWriterBuilder w;
        w["indentation"] = "";
        w["emitUTF8"] = true;
        req->setBody(Json::writeString(w, payload));

        // client захвачен в колбэк — живёт до ответа/таймаута.
        client->sendRequest(
            req,
            [cb, client](ReqResult result, const HttpResponsePtr& resp) {
                if (result != ReqResult::Ok || !resp)
                {
                    LOG_WARN << "ассистент: LLM недоступен (ReqResult "
                             << static_cast<int>(result) << ")";
                    cb(std::nullopt);
                    return;
                }
                const int status = static_cast<int>(resp->statusCode());
                auto reply = parseReply(status, std::string(resp->body()));
                if (!reply)
                    // Тело апстрима НЕ логируем: в нём id каталога/облака.
                    LOG_WARN << "ассистент: LLM ответил статусом " << status
                             << " без пригодного ответа";
                cb(std::move(reply));
            },
            kTimeoutSec);
    };
}

// --- Операции ----------------------------------------------------------------------

namespace {
const std::string kDeadlinesSql =
    "SELECT title, kind, due_at FROM deadlines "
    "WHERE (user_id = $1 OR (user_id IS NULL AND group_id = $2)) "
    "AND due_at > $3 ORDER BY due_at LIMIT 10";

const std::string kHistorySql =
    "SELECT id, role, content, created_at FROM assistant_messages "
    "WHERE user_id = $1 ORDER BY id DESC LIMIT $2";

const std::string kInsertPairSql =
    "INSERT INTO assistant_messages(user_id, role, content) VALUES "
    "($1, 'user', $2), ($3, 'assistant', $4)";

// Поиск аудитории для контекста — та же логика, что GET /v1/map/rooms?q=…
// (MapService::rooms): подстрочный LIKE по имени с экранированием.
const std::string kRoomSql =
    "SELECT r.name, b.name AS building, r.floor, r.kind FROM rooms r"
    " JOIN buildings b ON b.id = r.building_id"
    " WHERE r.name LIKE $1 ESCAPE '\\' ORDER BY r.id ASC LIMIT 5";

// Сборщик параллельных источников: cb стреляет ровно один раз — когда
// отработали все, либо при первой ошибке (fail-closed).
struct Gather
{
    AssistantService::ContextData data;
    AssistantService::ContextCb cb;
    std::atomic<int> pending{0};
    std::atomic<bool> failed{false};
    std::mutex mu;

    void done(bool ok)
    {
        if (!ok && !failed.exchange(true))
        {
            cb(false, AssistantService::ContextData{});
            return;
        }
        if (--pending == 0 && !failed.load())
            cb(true, std::move(data));
    }
};
}  // namespace

void AssistantService::collectContext(orm::DbClientPtr db,
                                      const SessionInfo& session,
                                      const std::string& today,
                                      const std::string& message,
                                      ContextCb cb)
{
    const int64_t userId = session.userId;
    ScheduleService::groupOfUser(
        db, userId,
        [db, session, today, message, userId, cb](
            ScheduleService::GroupLookup lookup,
            std::optional<int64_t> groupId) {
            if (lookup == ScheduleService::GroupLookup::Error)
            {
                cb(false, ContextData{});
                return;
            }
            auto g = std::make_shared<Gather>();
            g->cb = cb;
            g->data.today = today;
            g->data.events = Json::Value(Json::arrayValue);
            g->data.exams = Json::Value(Json::arrayValue);
            g->data.deadlines = Json::Value(Json::arrayValue);
            g->data.applications = Json::Value(Json::arrayValue);
            g->data.campus = Json::Value(Json::arrayValue);
            g->data.rooms = Json::Value(Json::arrayValue);
            const bool hasGroup = groupId.has_value();
            // «Проверь моё решение»: детерминированный маркер контекста —
            // слабой модели нужен явный запрет + наводящий вопрос. Слово
            // проверки + слово «решение/расчёт/задача», чтобы «проверь
            // расписание» маркер не включало. Регистронезависимо
            // («Проверь…» с заглавной — живой кейс 21.11).
            const std::string upperMsg = upperRu(message);
            const auto has = [&upperMsg](const char* needle) {
                return upperMsg.find(needle) != std::string::npos;
            };
            const bool checkWord = has("ПРОВЕРЬ") || has("ПРОВЕРИТЬ") ||
                                   has("ПРАВИЛЬНО ЛИ");
            const bool solutionWord = has("РЕШЕНИ") || has("РАСЧЁТ") ||
                                      has("РАСЧЕТ") || has("ПОСЧИТАЛ") ||
                                      has("ЗАДАЧ") || has("ПРИМЕР");
            g->data.solutionCheck = checkWord && solutionWord;
            // +1 источник, если в вопросе есть номер аудитории — ищем по
            // карте (GET /v1/map/rooms?q=…): «где аудитория 1207?» больше
            // не остаётся без данных портала.
            const std::string room = roomQuery(message);
            // Дата из вопроса («что за пары 25 ноября») → догрузить
            // расписание этой даты: сегодня/завтра уже в контексте, их
            // не дублируем (иначе модель видела «нет информации» на любой
            // день дальше завтра — живой кейс 21.11).
            std::string dateAsked = mentionedDate(message, today);
            if (dateAsked == today ||
                dateAsked == ScheduleService::addDays(today, 1))
                dateAsked.clear();
            const bool needDate = hasGroup && !dateAsked.empty();
            if (needDate)
                g->data.dateAsked = dateAsked;
            g->pending = (hasGroup ? 6 : 4) + (room.empty() ? 0 : 1) +
                         (needDate ? 1 : 0);
            const std::string now = nowIso();

            if (needDate)
                ScheduleService::eventsOnRange(
                    db, *groupId, dateAsked + "T00:00:00Z",
                    ScheduleService::addDays(dateAsked, 1) + "T00:00:00Z",
                    [g](bool ok, Json::Value events) {
                        if (ok)
                        {
                            std::lock_guard<std::mutex> l(g->mu);
                            g->data.dateEvents = std::move(events);
                        }
                        g->done(ok);
                    });

            if (hasGroup)
                *db << "SELECT name FROM groups WHERE id = $1"
                    << *groupId >>
                    [g, mentioned = mentionedGroup(message)](
                        const orm::Result& rows) {
                        if (!rows.empty())
                        {
                            std::lock_guard<std::mutex> l(g->mu);
                            g->data.group =
                                rows[0]["name"].as<std::string>();
                            // Чужая группа в вопросе → предупреждение в
                            // контексте (аудит 21.11: модель подменяла
                            // расписание пользователя).
                            if (!mentioned.empty() &&
                                mentioned != g->data.group)
                                g->data.foreignGroup = mentioned;
                        }
                        g->done(true);
                    } >>
                    [g](const DrogonDbException& e) {
                        LOG_ERROR << "AssistantService: группа: "
                                  << e.base().what();
                        g->done(false);
                    };

            if (!room.empty())
            {
                // Экранирование спецсимволов LIKE — как MapService::rooms.
                std::string like = "%";
                for (char c : room)
                {
                    if (c == '\\' || c == '%' || c == '_')
                        like += '\\';
                    like += c;
                }
                like += '%';
                *db << kRoomSql << like >>
                    [g](const orm::Result& rows) {
                        Json::Value arr(Json::arrayValue);
                        for (const auto& r : rows)
                        {
                            Json::Value x(Json::objectValue);
                            x["name"] = r["name"].as<std::string>();
                            x["building"] = r["building"].as<std::string>();
                            if (r["floor"].isNull())
                                x["floor"] = Json::Value();
                            else
                                x["floor"] = r["floor"].as<int64_t>();
                            if (r["kind"].isNull())
                                x["kind"] = Json::Value();
                            else
                                x["kind"] = r["kind"].as<std::string>();
                            arr.append(std::move(x));
                        }
                        {
                            std::lock_guard<std::mutex> l(g->mu);
                            g->data.rooms = std::move(arr);
                        }
                        g->done(true);
                    } >>
                    [g, room](const DrogonDbException& e) {
                        LOG_ERROR << "AssistantService: карта(" << room
                                  << "): " << e.base().what();
                        g->done(false);
                    };
            }

            if (hasGroup)
                ScheduleService::eventsOnRange(
                    // Окно с запасом вокруг сегодня+завтра; по дням
                    // раскладывает renderContext (isoDay).
                    db, *groupId,
                    ScheduleService::addDays(today, -1) + "T00:00:00Z",
                    ScheduleService::addDays(today, 3) + "T00:00:00Z",
                    [g](bool ok, Json::Value events) {
                        if (ok)
                        {
                            std::lock_guard<std::mutex> l(g->mu);
                            g->data.events = std::move(events);
                        }
                        g->done(ok);
                    });

            GradesService::exams(db, userId, [g](GradesService::Result r) {
                if (r.ok)
                {
                    std::lock_guard<std::mutex> l(g->mu);
                    g->data.exams = std::move(r.body["exams"]);
                }
                g->done(r.ok);
            });

            ApplicationsService::list(
                db, userId, std::nullopt,
                [g](ApplicationsService::Result r) {
                    if (r.ok)
                    {
                        Json::Value active(Json::arrayValue);
                        for (const auto& a : r.body["applications"])
                            if (DashboardService::isActiveApplicationStatus(
                                    a["status"].asString()))
                                active.append(a);
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.applications = std::move(active);
                    }
                    g->done(r.ok);
                });

            *db << kDeadlinesSql << userId << groupId.value_or(-1) << now >>
                [g](const orm::Result& rows) {
                    Json::Value arr(Json::arrayValue);
                    for (const auto& r : rows)
                    {
                        Json::Value x(Json::objectValue);
                        x["title"] = r["title"].as<std::string>();
                        x["kind"] = r["kind"].as<std::string>();
                        x["due_at"] = r["due_at"].as<std::string>();
                        arr.append(std::move(x));
                    }
                    {
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.deadlines = std::move(arr);
                    }
                    g->done(true);
                } >>
                [g, userId](const DrogonDbException& e) {
                    LOG_ERROR << "AssistantService: дедлайны(" << userId
                              << "): " << e.base().what();
                    g->done(false);
                };

            NewsService::feed(
                db, session, NewsService::Query{},
                [g, now](NewsService::FeedResult r) {
                    const bool ok = r.status == NewsService::FeedStatus::Ok;
                    if (ok)
                    {
                        // Лента — по published_at; в контекст — ближайшие
                        // 5 будущих событий по времени события.
                        std::vector<Json::Value> upcoming;
                        for (const auto& n : r.body["news"])
                            if (n["event_at"].isString() &&
                                n["event_at"].asString() >= now)
                                upcoming.push_back(n);
                        std::sort(upcoming.begin(), upcoming.end(),
                                  [](const Json::Value& a,
                                     const Json::Value& b) {
                                      return a["event_at"].asString() <
                                             b["event_at"].asString();
                                  });
                        Json::Value arr(Json::arrayValue);
                        for (std::size_t i = 0;
                             i < upcoming.size() && i < 5; ++i)
                            arr.append(upcoming[i]);
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.campus = std::move(arr);
                    }
                    g->done(ok);
                });
        });
}

void AssistantService::history(orm::DbClientPtr db,
                               int64_t userId,
                               int limit,
                               HistoryCb cb)
{
    *db << kHistorySql << userId << limit >>
        [cb](const orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            // DESC LIMIT → разворот: клиенту и в промт — по возрастанию.
            for (auto it = rows.rbegin(); it != rows.rend(); ++it)
            {
                const auto& r = *it;
                Json::Value m(Json::objectValue);
                m["id"] = r["id"].as<int64_t>();
                m["role"] = r["role"].as<std::string>();
                m["content"] = r["content"].as<std::string>();
                m["created_at"] = r["created_at"].as<std::string>();
                arr.append(std::move(m));
            }
            cb(true, std::move(arr));
        } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "AssistantService::history(" << userId
                      << "): " << e.base().what();
            cb(false, Json::Value(Json::arrayValue));
        };
}

// --- Предфильтр «вне компетенции» --------------------------------------------

namespace {

// Canned-ответ: тон модельных отказов («в портале нет такой информации» +
// куда смотреть), 1–2 предложения — по правилам промта.
const char* kOutOfScopeReply =
    "в портале нет такой информации. Я не решаю примеры и не перевожу "
    "тексты — помогаю по разделам портала: расписание, учёба, заявки, "
    "дедлайны.";

// Только ли арифметика в строке: цифры, операторы, скобки, точка/запятая
// (десятичные), «×»/«÷» (UTF-8 C3 97/B7). Любой иной символ (включая
// буквы) — не арифметика. Дефис-минус считается оператором, поэтому
// безбуквенная дата «2026-11-21» тоже заблокируется — такой запрос
// модель всё равно не ответила бы (нет в контексте).
bool isArithmeticOnly(const std::string& s)
{
    bool digit = false, op = false;
    for (std::size_t k = 0; k < s.size(); ++k)
    {
        const unsigned char c = static_cast<unsigned char>(s[k]);
        if (c >= 0x80)  // многобайтные: допускаем только «×»/«÷»
        {
            if (c == 0xC3 && k + 1 < s.size())
            {
                const unsigned char n =
                    static_cast<unsigned char>(s[k + 1]);
                if (n == 0x97 || n == 0xB7)
                {
                    op = true;
                    ++k;
                    continue;
                }
            }
            return false;
        }
        if (c >= '0' && c <= '9')
        {
            digit = true;
            continue;
        }
        if (c == ' ' || c == '\t')
            continue;
        switch (c)
        {
        case '+': case '-': case '*': case '/': case '^':
            op = true;
            break;
        case '(': case ')': case '.': case ',': case '%': case '=':
            break;  // структура выражения, не признак операции
        default:
            return false;
        }
    }
    return digit && op;
}

}  // namespace

bool AssistantService::isOutOfScopeRequest(const std::string& message)
{
    const std::string up = upperRu(message);
    // 1) Переводы — явная просьба в начале сообщения.
    static constexpr std::string_view kTranslations[] = {
        "ПЕРЕВЕДИ", "ПЕРЕВЕСТИ", "ПЕРЕВОД ", "TRANSLATE "};
    for (const auto p : kTranslations)
        if (up.compare(0, p.size(), p) == 0)
            return true;
    // 2) Чистая арифметика: срезать разговорный префикс и хвостовые
    //    знаки препинания («сколько будет 2+2?» → «2+2»).
    std::string s = up;
    static constexpr std::string_view kArithPrefixes[] = {
        "СКОЛЬКО БУДЕТ ", "ПОСЧИТАЙ ", "ВЫЧИСЛИ ", "ЧЕМУ РАВНО ",
        "ЧЕМУ РАВЕН ", "ЧЕМУ РАВНА "};
    for (const auto p : kArithPrefixes)
    {
        if (s.compare(0, p.size(), p) == 0)
        {
            s = s.substr(p.size());
            break;
        }
    }
    while (!s.empty() &&
           (s.back() == '?' || s.back() == '!' || s.back() == '.'))
        s.pop_back();
    if (!s.empty() && isArithmeticOnly(s))
        return true;
    // 3) Мат-функция + число («синус 30», «корень из 9», «cos 45»).
    //    Латинские SIN/COS/TAN — подстроки: риск задеть англ. слово
    //    с цифрой пренебрежим (язык портала русский).
    static constexpr std::string_view kMathWords[] = {
        "СИНУС", "КОСИНУС", "ТАНГЕНС", "КОТАНГЕНС", "ЛОГАРИФМ",
        "ФАКТОРИАЛ", "КОРЕНЬ", "SIN", "COS", "TAN", "SQRT", "LOG"};
    if (up.find_first_of("0123456789") != std::string::npos)
        for (const auto w : kMathWords)
            if (up.find(w) != std::string::npos)
                return true;
    return false;
}

void AssistantService::chat(orm::DbClientPtr db,
                            const SessionInfo& session,
                            const std::string& message,
                            const Config& cfg,
                            Transport transport,
                            ChatCb cb)
{
    if (!cfg.configured())
    {
        LOG_WARN << "ассистент не настроен (ASSISTANT_* в окружении)";
        cb(ChatResult{Status::Unavailable, {}});
        return;
    }
    const int64_t userId = session.userId;
    // Предфильтр «вне компетенции» (инцидент 21.11 — «5*3»): арифметика и
    // переводы не ходят в модель — canned-ответ, пара пишется в историю
    // как обычный обмен. Лимит контроллера запрос уже учёл.
    if (isOutOfScopeRequest(message))
    {
        const std::string text = kOutOfScopeReply;
        *db << kInsertPairSql << userId << message << userId << text >>
            [cb, text](const orm::Result&) {
                cb(ChatResult{Status::Ok, text});
            } >>
            [cb, userId](const DrogonDbException& e) {
                LOG_ERROR << "AssistantService: запись истории("
                          << userId << "): " << e.base().what();
                cb(ChatResult{Status::Error, {}});
            };
        return;
    }
    collectContext(
        db, session, todayUtc(), message,
        [db, userId, message, cfg, transport, cb](bool ok, ContextData data) {
            if (!ok)
            {
                cb(ChatResult{Status::Error, {}});
                return;
            }
            const std::string context = renderContext(data);
            history(
                db, userId, kHistoryForPrompt,
                [db, userId, message, cfg, transport, cb,
                 context](bool hok, Json::Value hist) {
                    if (!hok)
                    {
                        cb(ChatResult{Status::Error, {}});
                        return;
                    }
                    const Json::Value payload =
                        buildRequest(cfg, context, hist, message);
                    transport(
                        cfg, payload,
                        [db, userId, message, cb](
                            std::optional<std::string> reply) {
                            if (!reply)
                            {
                                cb(ChatResult{Status::Unavailable, {}});
                                return;
                            }
                            const std::string text = *reply;
                            *db << kInsertPairSql << userId << message
                                << userId << text >>
                                [cb, text](const orm::Result&) {
                                    cb(ChatResult{Status::Ok, text});
                                } >>
                                [cb, userId](const DrogonDbException& e) {
                                    LOG_ERROR << "AssistantService: запись "
                                                 "истории("
                                              << userId
                                              << "): " << e.base().what();
                                    cb(ChatResult{Status::Error, {}});
                                };
                        });
                });
        });
}

}  // namespace para
