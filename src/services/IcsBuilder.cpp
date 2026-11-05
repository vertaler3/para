#include "services/IcsBuilder.h"

namespace para {

std::string IcsBuilder::escapeText(const std::string& text)
{
    // RFC 5545 §3.3.11 (TEXT): backslash экранируется первым, затем
    // разделители ';' и ','; перевод строки (CRLF, LF или lone CR) → один
    // литерал "\n" (CRLF не даёт двух: '\n' после '\r' пропускается).
    std::string out;
    out.reserve(text.size());
    char prev = 0;
    for (const char c : text)
    {
        switch (c)
        {
            case '\\':
                out += "\\\\";
                break;
            case ';':
                out += "\\;";
                break;
            case ',':
                out += "\\,";
                break;
            case '\n':
                if (prev != '\r')
                    out += "\\n";
                break;
            case '\r':
                out += "\\n";
                break;
            default:
                out.push_back(c);
        }
        prev = c;
    }
    return out;
}

std::string IcsBuilder::toIcsDateTime(const std::string& iso)
{
    // Метки проекта — ISO 8601 UTC «2026-11-04T10:00:00Z»; формат ICS
    // DATE-TIME — базовый «20261104T100000Z» (RFC 5545 §3.3.5): убрать
    // '-' и ':', 'Z' остаётся (UTC).
    std::string out;
    out.reserve(16);
    for (const char c : iso)
    {
        if (c == '-' || c == ':')
            continue;
        out.push_back(c);
    }
    return out;
}

std::string IcsBuilder::summaryFor(const std::string& discipline,
                                   const std::string& room,
                                   const std::string& status,
                                   const std::optional<std::string>& prevRoom)
{
    // Предмет + аудитория + пометка статуса: перенос — с прежней аудиторией
    // («было → стало» видно прямо в календаре телефона, С1/С3).
    std::string s = discipline + " · ауд. " + room;
    if (status == "moved")
        s += " (перенос" +
             (prevRoom ? " из ауд. " + *prevRoom : std::string()) + ")";
    else if (status == "cancelled")
        s += " (отменено)";
    else if (status == "substituted")
        s += " (замена преподавателя)";
    return s;
}

std::string IcsBuilder::summaryForPlan(const std::string& kind,
                                       const std::string& discipline)
{
    // Виды session_plan — CHECK миграции 0002; подпись — как на экране
    // «Сессия» (С4). Неизвестный вид — просто предмет.
    if (kind == "consultation")
        return "Консультация: " + discipline;
    if (kind == "exam")
        return "Экзамен: " + discipline;
    if (kind == "credit")
        return "Зачёт: " + discipline;
    if (kind == "retake")
        return "Пересдача: " + discipline;
    return discipline;
}

std::string IcsBuilder::build(const std::vector<CalendarItem>& items,
                              const std::string& dtstamp)
{
    // Все строки — CRLF (RFC 5545 §2.1). PRODID — ASCII (данные вымышленные,
    // AGENTS.md §5.1); CALSCALE — григорианский по заданию шага.
    std::string out;
    out += "BEGIN:VCALENDAR\r\n";
    out += "VERSION:2.0\r\n";
    out += "PRODID:-//para//demo koncept dlya haketona//RU\r\n";
    out += "CALSCALE:GREGORIAN\r\n";
    for (const CalendarItem& item : items)
    {
        out += "BEGIN:VEVENT\r\n";
        out += "UID:" + escapeText(item.uid) + "\r\n";
        out += "DTSTAMP:" + dtstamp + "\r\n";
        out += "DTSTART:" + toIcsDateTime(item.startIso) + "\r\n";
        out += "DTEND:" + toIcsDateTime(item.endIso) + "\r\n";
        out += "SUMMARY:" + escapeText(item.summary) + "\r\n";
        out += "LOCATION:" + escapeText(item.location) + "\r\n";
        out += "END:VEVENT\r\n";
    }
    out += "END:VCALENDAR\r\n";
    return out;
}

}  // namespace para
