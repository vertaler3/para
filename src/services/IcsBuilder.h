#pragma once

#include <optional>
#include <string>
#include <vector>

namespace para {

// Сборка VCALENDAR по RFC 5545 для приватной подписки календаря (идея 5.4;
// 02 §5.2: GET /ics/{token} → 200 text/calendar). Чистые функции БЕЗ БД и
// времени — детерминированы и юнит-тестируются (структура, CRLF,
// экранирование, пустой набор); источники и SQL — IcsService.
// Строки — CRLF (RFC 5545 §2.1); длинные строки НЕ фолдятся по 75 октетам:
// кириллица многобайтна, фолдинг — риск сломать клиенты, для прототипа
// сознательно опущен (клиенты принимают unfolded).
class IcsBuilder
{
  public:
    // Одна запись календаря: занятие группы или персональная дата сессии.
    struct CalendarItem
    {
        std::string uid;       // глобально уникальный, стабилен между сборками
        std::string startIso;  // ISO 8601 UTC проекта: 2026-11-04T10:00:00Z
        std::string endIso;
        std::string summary;   // неэкранированный текст
        std::string location;  // аудитория (неэкранированная)
    };

    // Экранирование TEXT по RFC 5545 §3.3.11: сначала обратный слеш, затем
    // точка с запятой и запятая, переводы строки → литерал \n.
    static std::string escapeText(const std::string& text);

    // ISO-метка проекта → формат ICS DATE-TIME UTC: 20261104T100000Z.
    static std::string toIcsDateTime(const std::string& iso);

    // SUMMARY пары: предмет + аудитория + пометка статуса («было → стало»
    // видно в календаре: перенос — со старой аудиторией).
    static std::string summaryFor(const std::string& discipline,
                                  const std::string& room,
                                  const std::string& status,
                                  const std::optional<std::string>& prevRoom);

    // SUMMARY персональной даты сессии (экран «Сессия», С4).
    static std::string summaryForPlan(const std::string& kind,
                                      const std::string& discipline);

    // Готовый VCALENDAR: заголовки + VEVENT + END:VCALENDAR, все строки CRLF.
    // dtstamp — текущий момент в формате ICS (20261105T201830Z) — задаёт
    // вызывающий, чтобы сборка оставалась чистой функцией.
    static std::string build(const std::vector<CalendarItem>& items,
                             const std::string& dtstamp);
};

}  // namespace para
