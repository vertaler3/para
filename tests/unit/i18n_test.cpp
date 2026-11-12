// Юнит-тесты словаря интерфейса 5.18 (фича «Мультиязычный UI: ru/en/中文»,
// шаг 7; словарь — src/models/i18n.h, шаг 1, резолв — src/models/i18n.cpp):
//  - t(): ключи-представители ВСЕХ категорий шага 1 в трёх словарях
//    (точные значения ru/en/zh — дословные UTF-8-литералы);
//  - t(): полный keyset рантайм-перебором — двоичный поиск lookupIn по
//    каждому ключу kRu во всех трёх таблицах (полноту keyset на компиляции
//    держат static_assert sameKeyset в i18n.h; здесь — сам поиск);
//  - t(): отсутствующий ключ → пустая string_view (нет и в ru — показывать
//    нечего, договорённость шапки i18n.h); Lang вне ru/en/zh (битое значение
//    enum) → ru-таблица («неизвестный язык — русский», инвариант фичи).
//    Замечание о фолбэке en/zh→ru для отсутствующего в таблице языка ключа:
//    на полном keyset он недостижим (sameKeyset) — проверены обе наблюдаемые
//    стороны контракта: «дыр нет» (полный перебор выше) и «нет нигде →
//    пусто»;
//  - резолв языка запроса: cookie para_lang приоритетнее Accept-Language,
//    невалидное cookie → заголовок, ни того ни другого → ru (defaultLang —
//    «русский — дефолт без указаний»);
//  - langFromAcceptLanguage: RFC 7231 §5.3.1 — зоны (zh-CN/en-US), q-веса,
//    q=0 «не приемлем», «*», мусорный q, регистр, «_»-подтег.
// Словарь не зависит от drogon (шапка models/i18n.h) — тест линкуется
// только с src/models/i18n.cpp, без БД и HttpViewData (мост I18nView
// проверяет рендер-экзешник шага 7 — паттерн Э7, в репо не попадает).
#include <string_view>

#include <doctest.hpp>

#include "models/i18n.h"

using namespace para::i18n;

namespace {

struct Rep
{
    std::string_view key;
    std::string_view ru;
    std::string_view en;
    std::string_view zh;
};

// По одному ключу из КАЖДОЙ категории шага 1 (состав категорий — префиксы
// таблиц kRu/kEn/kZh: a11y/app/apps/campus/common/control/dash/error/exam/
// grades/lang/lesson/login/month/nav/news/notif/offline/onboarding/plural/
// profile/schedule/settings/staff/toast/weekday/weekdayShort). Значения —
// дословно из таблиц (сверка с фактическим словарём, не «похожие строки»).
constexpr Rep kReps[]{
    {"a11y.skipToContent", "К содержимому", "Skip to content", "跳到主要内容"},
    {"app.title", "Пара", "Para", "Para"},
    {"apps.title", "Мои заявки", "My requests", "我的申请"},
    {"campus.title", "В кампусе", "On campus", "校园服务"},
    {"common.retry", "Повторить", "Retry", "重试"},
    {"control.exam", "Экзамен", "Exam", "考试"},
    {"dash.heading", "Всё важное — на один экран.",
     "Everything that matters, on one screen.", "要事一览，尽在一屏。"},
    {"error.loadSchedule", "Не получилось загрузить расписание",
     "Couldn't load the schedule", "无法加载课程表"},
    {"exam.retake", "Пересдача", "Retake", "补考"},
    {"grades.session", "Сессия", "Exam period", "考试周"},
    // автонимы сегмента языка НЕ переводятся — одинаковы во всех таблицах
    {"lang.zh", "中文", "中文", "中文"},
    {"lesson.lecture", "Лекция", "Lecture", "讲授课"},
    {"login.submit", "Войти", "Sign in", "登录"},
    {"month.11", "ноября", "November", "十一月"},
    {"nav.schedule", "Расписание", "Schedule", "课程表"},
    {"news.one", "Новость", "News", "新闻"},
    {"notif.title", "Уведомления", "Notifications", "通知"},
    {"offline.banner", "Нет сети. Проверьте подключение — и попробуйте ещё раз.",
     "No network. Check your connection and try again.",
     "无网络。请检查连接后重试。"},
    {"onboarding.skip", "Пропустить", "Skip", "跳过"},
    {"plural.minuteOne", "минуту", "minute", "分钟"},
    {"profile.language", "Язык", "Language", "语言"},
    {"schedule.sheetTitle", "Детали пары", "Class details", "课程详情"},
    {"settings.quietHours", "Тихие часы", "Quiet hours", "免打扰时段"},
    {"staff.desk", "Стол сотрудника", "Staff desk", "员工工作台"},
    {"toast.netBack", "Сеть вернулась — данные обновлены",
     "Back online — data updated", "网络已恢复，数据已更新"},
    {"weekday.01", "Понедельник", "Monday", "周一"},
    {"weekdayShort.03", "Ср", "We", "周三"},
};

}  // namespace

TEST_CASE("i18n: t() — ключи всех категорий шага 1 в трёх словарях")
{
    for (const auto& rep : kReps)
    {
        CAPTURE(rep.key);
        CHECK(t(rep.key, Lang::Ru) == rep.ru);
        CHECK(t(rep.key, Lang::En) == rep.en);
        CHECK(t(rep.key, Lang::Zh) == rep.zh);
    }
}

TEST_CASE("i18n: t() — полный keyset, рантайм-перебор трёх таблиц")
{
    // Размеры равны (компиляции это держит sameKeyset; здесь — факт, на
    // котором построена вставка I18nView::insertUiStrings «по индексу i»).
    const std::size_t n = std::size(kRu);
    REQUIRE(std::size(kEn) == n);
    REQUIRE(std::size(kZh) == n);
    // Каждый ключ ru-таблицы (источник истины keyset) резолвится t() во
    // всех трёх языках в точности в значение своей таблицы — прогон
    // двоичного поиска lookupIn по всему словарю; заодно «дыр» нет:
    // фолбэк en/zh→ru не срабатывает ни на одном ключе (значения непусты).
    for (std::size_t i = 0; i < n; ++i)
    {
        CAPTURE(kRu[i].key);
        CHECK_FALSE(kRu[i].value.empty());
        CHECK(t(kRu[i].key, Lang::Ru) == kRu[i].value);
        CHECK(t(kRu[i].key, Lang::En) == kEn[i].value);
        CHECK(t(kRu[i].key, Lang::Zh) == kZh[i].value);
    }
}

TEST_CASE("i18n: t() — отсутствующий ключ и неизвестный Lang")
{
    // Ключа нет нигде (в т.ч. в ru) → пустая string_view: показывать
    // нечего, вызывающий оставляет ключ/сырой текст (шапка i18n.h).
    // Такой ключ один и тот же для всех языков — ru-фолбэк не может
    // вернуть значение, которого нет и в ru.
    CHECK(t("no.such.key", Lang::Ru).empty());
    CHECK(t("no.such.key", Lang::En).empty());
    CHECK(t("no.such.key", Lang::Zh).empty());
    CHECK(t("", Lang::En).empty());  // пустой ключ — тоже «нет ключа»

    // «Неизвестный lang → ru»: значение enum вне ru/en/zh (ошибочный
    // резолв/битые данные) не совпадает ни с одним case — t() доходит до
    // ru-таблицы. Проверка той же ru-строки, что и для Lang::Ru.
    const Lang bogus = static_cast<Lang>(77);
    CHECK(t("nav.schedule", bogus) == t("nav.schedule", Lang::Ru));
    CHECK(t("nav.schedule", bogus) == std::string_view("Расписание"));
    CHECK(t("offline.banner", static_cast<Lang>(200)) ==
          std::string_view("Нет сети. Проверьте подключение — и попробуйте "
                           "ещё раз."));
}

TEST_CASE("i18n: коды Lang — langCode/langFromCode/defaultLang")
{
    CHECK(langCode(Lang::Ru) == "ru");
    CHECK(langCode(Lang::En) == "en");
    CHECK(langCode(Lang::Zh) == "zh");
    CHECK(defaultLang() == Lang::Ru);  // русский — язык по умолчанию (5.18)

    REQUIRE(langFromCode("ru").has_value());
    CHECK(*langFromCode("ru") == Lang::Ru);
    REQUIRE(langFromCode("en").has_value());
    CHECK(*langFromCode("en") == Lang::En);
    REQUIRE(langFromCode("zh").has_value());
    CHECK(*langFromCode("zh") == Lang::Zh);
    // только точные коды: зоны/регистр/мусор → nullopt (не ru!)
    CHECK_FALSE(langFromCode("en-US").has_value());
    CHECK_FALSE(langFromCode("zh-CN").has_value());
    CHECK_FALSE(langFromCode("RU").has_value());
    CHECK_FALSE(langFromCode("de").has_value());
    CHECK_FALSE(langFromCode("").has_value());
}

TEST_CASE("i18n: langFromCookie — точные коды, пробелы, мусор")
{
    REQUIRE(langFromCookie("ru").has_value());
    CHECK(*langFromCookie("ru") == Lang::Ru);
    REQUIRE(langFromCookie("en").has_value());
    CHECK(*langFromCookie("en") == Lang::En);
    REQUIRE(langFromCookie("zh").has_value());
    CHECK(*langFromCookie("zh") == Lang::Zh);
    // пробелы по краям прощаются (cookie-значение могло быть записано с ними)
    REQUIRE(langFromCookie(" en ").has_value());
    CHECK(*langFromCookie(" en ") == Lang::En);
    CHECK(*langFromCookie("\ten\n") == Lang::En);
    // прочее → nullopt: резолв уходит в Accept-Language (не «мусор → ru»,
    // а «мусор → следующий источник» — ru только если и заголовок молчит)
    CHECK_FALSE(langFromCookie("").has_value());
    CHECK_FALSE(langFromCookie("de").has_value());
    CHECK_FALSE(langFromCookie("ru-RU").has_value());
    CHECK_FALSE(langFromCookie("RU").has_value());
    CHECK_FALSE(langFromCookie("мусор").has_value());
}

TEST_CASE("i18n: langFromAcceptLanguage — RFC 7231: зоны, q, «*», мусор")
{
    // один язык / зона (основной подтег до «-») / регистр
    REQUIRE(langFromAcceptLanguage("en").has_value());
    CHECK(*langFromAcceptLanguage("en") == Lang::En);
    CHECK(*langFromAcceptLanguage("en-US") == Lang::En);
    CHECK(*langFromAcceptLanguage("EN-US,en") == Lang::En);
    CHECK(*langFromAcceptLanguage("zh-CN,zh;q=0.9,en") == Lang::Zh);
    CHECK(*langFromAcceptLanguage("ZH") == Lang::Zh);
    CHECK(*langFromAcceptLanguage("ru;q=0.8, ru-RU") == Lang::Ru);
    // «_» — тоже разрыв подтега (клиенты пишут и так)
    CHECK(*langFromAcceptLanguage("en_US") == Lang::En);
    // выбор среди поддерживаемых: наибольший q; при равенстве — первый
    CHECK(*langFromAcceptLanguage("fr-CA,fr;q=0.9,en;q=0.5") == Lang::En);
    CHECK(*langFromAcceptLanguage("zh;q=0.8, en;q=0.9") == Lang::En);
    CHECK(*langFromAcceptLanguage("en;q=0.5, ru;q=0.5") == Lang::En);
    CHECK(*langFromAcceptLanguage("ru;q=0.5, en;q=0.5") == Lang::Ru);
    // без q — вес 1 (максимум); дробные/начинающиеся с точки — валидны
    CHECK(*langFromAcceptLanguage("de, ru") == Lang::Ru);
    CHECK(*langFromAcceptLanguage("de;q=.9, zh;q=.8") == Lang::Zh);
    // q=0 — «не приемлем», не выбирается; следующий поддерживаемый — да
    CHECK(*langFromAcceptLanguage("en;q=0, ru") == Lang::Ru);
    CHECK(*langFromAcceptLanguage("en;q=0.0, zh") == Lang::Zh);
    // «*» язык не называет — на него не резолвим; мусорный q = нет q
    CHECK(*langFromAcceptLanguage("*, en;q=0.5") == Lang::En);
    CHECK(*langFromAcceptLanguage("en;q=abc, ru") == Lang::En);
    // нет поддерживаемых записей → nullopt (→ resolveLang даст ru)
    CHECK_FALSE(langFromAcceptLanguage("").has_value());
    CHECK_FALSE(langFromAcceptLanguage("de").has_value());
    CHECK_FALSE(langFromAcceptLanguage("fr-CA,fr;q=0.9").has_value());
    CHECK_FALSE(langFromAcceptLanguage("*").has_value());
    CHECK_FALSE(langFromAcceptLanguage("en;q=0").has_value());
    // только поддерживаемые с q=0 — тоже «ничего не приемлемо»
    CHECK_FALSE(langFromAcceptLanguage("en;q=0, ru;q=0, zh;q=0.0")
                    .has_value());
}

TEST_CASE("i18n: resolveLang — cookie приоритетнее Accept-Language; "
          "невалидные → ru")
{
    // cookie para_lang побеждает заголовок (явный выбор сегмента в профиле)
    CHECK(resolveLang("en", "ru,ru-RU;q=0.9") == Lang::En);
    CHECK(resolveLang("zh", "en-US,en;q=0.9") == Lang::Zh);
    CHECK(resolveLang("ru", "zh-CN,zh;q=0.9") == Lang::Ru);
    // невалидное cookie — НЕ ru, а следующий источник: Accept-Language
    CHECK(resolveLang("de", "en") == Lang::En);
    CHECK(resolveLang("", "zh-CN,zh;q=0.9,en") == Lang::Zh);
    CHECK(resolveLang("ru-RU", "en-US,en;q=0.8,ru;q=0.1") == Lang::En);
    CHECK(resolveLang("мусор", "ru;q=0.5,en") == Lang::En);
    // без q вес максимален: ru (запись без q) выигрывает у en;q=0.9
    CHECK(resolveLang("мусор", "en;q=0.9,ru") == Lang::Ru);
    // нет ничего валидного нигде → русский (defaultLang, инвариант 5.18:
    // «русский — дефолт без указаний»)
    CHECK(resolveLang("", "") == Lang::Ru);
    CHECK(resolveLang("de", "fr") == Lang::Ru);
    CHECK(resolveLang("", "*;q=1") == Lang::Ru);
    CHECK(resolveLang("", "de,fr-CA;q=0.8") == Lang::Ru);
}
