// Юнит-тесты IcsBuilder (Э3 шаг 2; RFC 5545): структура VCALENDAR,
// переводы строк CRLF, экранирование спецсимволов TEXT, набор без событий,
// формат DATE-TIME и подписи SUMMARY. IcsBuilder — чистые функции (без БД
// и wall-clock: DTSTAMP задаёт вызывающий) — тест детерминирован.
#include <string>
#include <vector>

#include <doctest.hpp>

#include "services/IcsBuilder.h"

using para::IcsBuilder;

namespace {

// Считаем, что каждая '\n' предшествуется '\r' (все строки — CRLF).
bool allCrlf(const std::string& s)
{
    for (std::size_t i = 0; i < s.size(); ++i)
        if (s[i] == '\n' && (i == 0 || s[i - 1] != '\r'))
            return false;
    return true;
}

int countOf(const std::string& s, const std::string& needle)
{
    int n = 0;
    for (std::size_t pos = s.find(needle); pos != std::string::npos;
         pos = s.find(needle, pos + needle.size()))
        ++n;
    return n;
}

IcsBuilder::CalendarItem demoItem()
{
    return IcsBuilder::CalendarItem{"para-event-5@para.demo",
                                    "2026-11-04T10:00:00Z",
                                    "2026-11-04T11:30:00Z",
                                    "Мировая экономика · ауд. 4210 (перенос)",
                                    "4210"};
}

}  // namespace

TEST_CASE("IcsBuilder: структура VCALENDAR по RFC 5545")
{
    const std::string ics =
        IcsBuilder::build({demoItem()}, "20261105T201830Z");

    CHECK(ics.rfind("BEGIN:VCALENDAR\r\n", 0) == 0);  // первая строка
    CHECK(ics.find("VERSION:2.0\r\n") != std::string::npos);
    CHECK(ics.find("PRODID:-//para//demo koncept dlya haketona//RU\r\n") !=
          std::string::npos);
    CHECK(ics.find("CALSCALE:GREGORIAN\r\n") != std::string::npos);
    CHECK(ics.find("BEGIN:VEVENT\r\n") != std::string::npos);
    CHECK(ics.find("UID:para-event-5@para.demo\r\n") != std::string::npos);
    CHECK(ics.find("DTSTAMP:20261105T201830Z\r\n") != std::string::npos);
    CHECK(ics.find("DTSTART:20261104T100000Z\r\n") != std::string::npos);
    CHECK(ics.find("DTEND:20261104T113000Z\r\n") != std::string::npos);
    CHECK(ics.find("SUMMARY:Мировая экономика · ауд. 4210 (перенос)\r\n") !=
          std::string::npos);
    CHECK(ics.find("LOCATION:4210\r\n") != std::string::npos);
    CHECK(ics.find("END:VEVENT\r\n") != std::string::npos);
    CHECK(ics.ends_with("END:VCALENDAR\r\n"));  // последняя строка
    CHECK(countOf(ics, "BEGIN:VEVENT") == 1);  // один item → один VEVENT
    // Порядок: END:VCALENDAR — последний непустой текст.
    CHECK(ics.find("END:VCALENDAR") > ics.find("END:VEVENT"));
}

TEST_CASE("IcsBuilder: все строки — CRLF, без одиночных LF")
{
    const auto ics =
        IcsBuilder::build({demoItem(), demoItem()}, "20261105T201830Z");
    CHECK(allCrlf(ics));
    // 4 строки заголовка + 8 строк на VEVENT * 2 + END:VCALENDAR = 21.
    CHECK(countOf(ics, "\r\n") == 21);
    CHECK(countOf(ics, "\n") == 21);
    CHECK(countOf(ics, "\r") == 21);
}

TEST_CASE("IcsBuilder: экранирование TEXT по RFC 5545 §3.3.11")
{
    // Порядок: backslash первым; ';' и ',' экранируются; LF → "\n" (литерал).
    CHECK(IcsBuilder::escapeText("a,b") == "a\\,b");
    CHECK(IcsBuilder::escapeText("a;b") == "a\\;b");
    CHECK(IcsBuilder::escapeText("a\\b") == "a\\\\b");
    CHECK(IcsBuilder::escapeText(std::string("a\nb")) == "a\\nb");
    CHECK(IcsBuilder::escapeText(std::string("a\r\nb")) == "a\\nb");  // CRLF → \n
    CHECK(IcsBuilder::escapeText(std::string("a\rb")) == "a\\nb");    // lone CR → \n
    CHECK(IcsBuilder::escapeText("просто текст") == "просто текст");

    // В сборке: спецсимволы в SUMMARY/LOCATION попадают в текст экранированными.
    IcsBuilder::CalendarItem tricky{"uid,5@para.demo",
                                    "2026-11-04T10:00:00Z",
                                    "2026-11-04T11:30:00Z",
                                    "Физика, ч. 2; лекция\nвторая строка",
                                    "ауд. 42,10; корпус Б"};
    const std::string ics = IcsBuilder::build({tricky}, "20261105T201830Z");
    CHECK(ics.find(
              "SUMMARY:Физика\\, ч. 2\\; лекция\\nвторая строка\r\n") !=
          std::string::npos);
    CHECK(ics.find("LOCATION:ауд. 42\\,10\\; корпус Б\r\n") !=
          std::string::npos);
    CHECK(ics.find("UID:uid\\,5@para.demo\r\n") != std::string::npos);
    // Экранирование не ломает CRLF-структуру.
    CHECK(allCrlf(ics));
}

TEST_CASE("IcsBuilder: DATE-TIME — формат базовый UTC")
{
    CHECK(IcsBuilder::toIcsDateTime("2026-11-04T10:00:00Z") ==
          "20261104T100000Z");
    CHECK(IcsBuilder::toIcsDateTime("2026-11-18T14:05:30Z") ==
          "20261118T140530Z");
    // Уже в базовом формате — без изменений (идемпотентность).
    CHECK(IcsBuilder::toIcsDateTime("20261104T100000Z") == "20261104T100000Z");
}

TEST_CASE("IcsBuilder: SUMMARY — предмет + аудитория + пометка статуса")
{
    using para::IcsBuilder;
    // normal: без пометки.
    CHECK(IcsBuilder::summaryFor("Мировая экономика", "3024", "normal",
                                 std::nullopt) ==
          "Мировая экономика · ауд. 3024");
    // moved: «было → стало» — прежняя аудитория в подписи.
    CHECK(IcsBuilder::summaryFor("Мировая экономика", "4210", "moved",
                                 std::string("3024")) ==
          "Мировая экономика · ауд. 4210 (перенос из ауд. 3024)");
    // cancelled/substituted: пометка статуса.
    CHECK(IcsBuilder::summaryFor("История МЭО", "2109", "cancelled",
                                 std::string("2109")) ==
          "История МЭО · ауд. 2109 (отменено)");
    CHECK(IcsBuilder::summaryFor("История России", "1201", "substituted",
                                 std::nullopt) ==
          "История России · ауд. 1201 (замена преподавателя)");

    // Персональные даты сессии (kind из CHECK миграции 0002).
    CHECK(IcsBuilder::summaryForPlan("consultation", "Мировая экономика") ==
          "Консультация: Мировая экономика");
    CHECK(IcsBuilder::summaryForPlan("exam", "История МЭО") ==
          "Экзамен: История МЭО");
    CHECK(IcsBuilder::summaryForPlan("retake", "История МЭО") ==
          "Пересдача: История МЭО");
    CHECK(IcsBuilder::summaryForPlan("credit", "Иностранный язык") ==
          "Зачёт: Иностранный язык");
    CHECK(IcsBuilder::summaryForPlan("unknown", "X") == "X");
}

TEST_CASE("IcsBuilder: набор без событий — валидный пустой календарь")
{
    const std::string ics = IcsBuilder::build({}, "20261105T201830Z");
    CHECK(ics.find("BEGIN:VCALENDAR") == 0);
    CHECK(ics.find("VERSION:2.0") != std::string::npos);
    CHECK(ics.find("CALSCALE:GREGORIAN") != std::string::npos);
    CHECK(ics.ends_with("END:VCALENDAR\r\n"));  // последняя строка
    CHECK(countOf(ics, "BEGIN:VEVENT") == 0);
    CHECK(countOf(ics, "END:VEVENT") == 0);
    CHECK(allCrlf(ics));
    // Ровно 5 строк: 4 заголовка + END:VCALENDAR.
    CHECK(countOf(ics, "\r\n") == 5);
}
