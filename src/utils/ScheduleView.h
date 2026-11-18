#pragma once

// Общая вёрстка экрана «Расписание» дизайна v3 (fe_rebuild, шаги 3–4):
// партиалы views/partials/schedule_today.csp и schedule_week.csp рисуют
// карточку пары и блок изменения одинаково — разметка эталона
// design-drafts/v3/pages/schedule.html / schedule-week.html живёт здесь, а не
// копией в двух шаблонах. Header-only: подключается из сгенерированных
// drogon_ctl партиалов (`<%inc #include "utils/ScheduleView.h" %>`), src/ — в
// include-путях целей para и security_tests.
//
// Данные — объекты события ScheduleService::eventJson (02 §5.2); строки
// интерфейса — словарь 5.18 через Tr (t.<key> из view-данных, Р-16);
// пользовательские строки (дисциплина, аудитория, корпус) экранируются
// htmlTranslate (02 §6.4). Время — «настенное» из строки «…Z» (substr), как во
// всех шаблонах расписания.

#include <drogon/HttpViewData.h>
#include <drogon/utils/Utilities.h>
#include <json/json.h>

#include <algorithm>
#include <functional>
#include <string>

namespace para::schedview
{

// Строка словаря по ключу без префикса «t.».
using Tr = std::function<std::string(const std::string&)>;

struct Ctx
{
    Tr t;
    std::string lang;  // ru | en | zh
};

inline std::string esc(const std::string& s)
{
    return drogon::HttpViewData::htmlTranslate(s);
}

inline std::string str(const Json::Value& e, const char* key)
{
    const Json::Value& v = e[key];
    return v.isString() ? v.asString() : std::string();
}

inline std::string timeOf(const std::string& iso)
{
    return iso.size() >= 16 ? iso.substr(11, 5) : std::string("--:--");
}

inline std::string dateOf(const std::string& iso)
{
    return iso.size() >= 10 ? iso.substr(0, 10) : std::string();
}

// Минуты от полуночи «настенного» времени; -1 — нет времени.
inline int minutesOf(const std::string& iso)
{
    if (iso.size() < 16 || iso[13] != ':')
        return -1;
    try
    {
        return std::stoi(iso.substr(11, 2)) * 60 + std::stoi(iso.substr(14, 2));
    }
    catch (...)
    {
        return -1;
    }
}

// --- календарная арифметика без <ctime> (days from civil) -----------------
inline long daysFromCivil(int y, int m, int d)
{
    y -= m <= 2;
    const long era = (y >= 0 ? y : y - 399) / 400;
    const long yoe = y - era * 400;
    const long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097L + doe - 719468L;
}

inline bool splitYmd(const std::string& ymd, int& y, int& m, int& d)
{
    y = m = d = 0;
    if (ymd.size() < 10)
        return false;
    try
    {
        y = std::stoi(ymd.substr(0, 4));
        m = std::stoi(ymd.substr(5, 2));
        d = std::stoi(ymd.substr(8, 2));
    }
    catch (...)
    {
        return false;
    }
    return m >= 1 && m <= 12 && d >= 1 && d <= 31;
}

inline long daysOf(const std::string& ymd)
{
    int y = 0, m = 0, d = 0;
    return splitYmd(ymd, y, m, d) ? daysFromCivil(y, m, d) : 0;
}

inline std::string ymdFromDays(long z)
{
    z += 719468L;
    const long era = (z >= 0 ? z : z - 146096L) / 146097L;
    const long doe = z - era * 146097L;
    const long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long yy = yoe + era * 400;
    const long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long mp = (5 * doy + 2) / 153;
    const int d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    const int m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    const int y = static_cast<int>(yy + (m <= 2));
    const auto pad2 = [](int v) {
        return (v < 10 ? "0" : "") + std::to_string(v);
    };
    return std::to_string(y) + "-" + pad2(m) + "-" + pad2(d);
}

// 0 = понедельник … 6 = воскресенье; -1 — не дата.
inline int weekdayOf(const std::string& ymd)
{
    int y = 0, m = 0, d = 0;
    if (!splitYmd(ymd, y, m, d))
        return -1;
    const long z = daysFromCivil(y, m, d);
    return static_cast<int>(((z + 3) % 7 + 7) % 7);
}

// Понедельник недели, содержащей ymd.
inline std::string mondayOf(const std::string& ymd)
{
    const int w = weekdayOf(ymd);
    return w < 0 ? ymd : ymdFromDays(daysOf(ymd) - w);
}

// --- локализованные подписи (словарь t.month.NN / weekday.0N) -------------
inline std::string twoDigit(int v)
{
    return (v < 10 ? "0" : "") + std::to_string(v);
}

// «17 ноября» / «17 November» / «11月17日».
inline std::string dayMonth(const Ctx& c, int d, int m)
{
    if (m < 1 || m > 12)
        return "";
    if (c.lang == "zh")
        return std::to_string(m) + "月" + std::to_string(d) + "日";
    return std::to_string(d) + " " + c.t("month." + twoDigit(m));
}

inline std::string weekdayName(const Ctx& c, int w)
{
    return w < 0 || w > 6 ? std::string()
                          : c.t("weekday.0" + std::to_string(w + 1));
}

inline std::string weekdayShort(const Ctx& c, int w)
{
    return w < 0 || w > 6 ? std::string()
                          : c.t("weekdayShort.0" + std::to_string(w + 1));
}

// «Среда, 4 ноября» (full) / «Ср, 4 ноября» (short).
inline std::string dayTitle(const Ctx& c, const std::string& ymd,
                            bool shortName = false)
{
    const int w = weekdayOf(ymd);
    std::string out = shortName ? weekdayShort(c, w) : weekdayName(c, w);
    int y = 0, m = 0, d = 0;
    if (splitYmd(ymd, y, m, d))
    {
        const std::string dm = dayMonth(c, d, m);
        if (!dm.empty())
            out += (c.lang == "zh" ? std::string("，") : std::string(", ")) + dm;
    }
    return out;
}

// «2–8 ноября 2026» / «30 октября – 5 ноября 2026» / zh «11月2–8日 2026».
inline std::string weekRange(const Ctx& c, const std::string& f,
                             const std::string& t)
{
    int fy = 0, fm = 0, fd = 0, ty = 0, tm = 0, td = 0;
    if (!splitYmd(f, fy, fm, fd) || !splitYmd(t, ty, tm, td))
        return std::string();
    if (c.lang == "zh")
    {
        if (fm == tm)
            return std::to_string(fm) + "月" + std::to_string(fd) + "–" +
                   std::to_string(td) + "日 " + std::to_string(ty);
        return dayMonth(c, fd, fm) + "–" + dayMonth(c, td, tm) + " " +
               std::to_string(ty);
    }
    std::string out = std::to_string(fd);
    if (fm != tm)
        out += " " + c.t("month." + twoDigit(fm));
    out += fm != tm ? " – " : "–";
    out += std::to_string(td) + " " + c.t("month." + twoDigit(tm)) + " " +
           std::to_string(ty);
    return out;
}

// «Ноябрь 2026» / через месяц — «Октябрь – Ноябрь 2026».
inline std::string monthTitle(const Ctx& c, const std::string& f,
                              const std::string& t)
{
    int fy = 0, fm = 0, fd = 0, ty = 0, tm = 0, td = 0;
    if (!splitYmd(f, fy, fm, fd) || !splitYmd(t, ty, tm, td))
        return std::string();
    std::string out = c.t("monthNom." + twoDigit(fm));
    if (fm != tm)
        out += " – " + c.t("monthNom." + twoDigit(tm));
    return out + " " + std::to_string(ty);
}

inline std::string kindLabel(const Ctx& c, const std::string& k)
{
    if (k == "lecture" || k == "seminar" || k == "practice")
        return c.t("lesson." + k);
    return "";
}

// «3 занятия»: ru — три формы, en/zh — единственное только при n == 1.
inline std::string countLabel(const Ctx& c, int n, const std::string& base)
{
    std::string form = "Many";
    if (c.lang == "ru")
    {
        const int m10 = n % 10, m100 = n % 100;
        if (m10 == 1 && m100 != 11)
            form = "One";
        else if (m10 >= 2 && m10 <= 4 && (m100 < 12 || m100 > 14))
            form = "Few";
    }
    else if (n == 1)
    {
        form = "One";
    }
    return std::to_string(n) + " " + c.t("plural." + base + form);
}

inline std::string mapHref(const std::string& room)
{
    return "/map?q=" + esc(drogon::utils::urlEncodeComponent(room));
}

// «4210 · Корпус Б» (корпус — поле building, если есть).
inline std::string roomWithBuilding(const std::string& room,
                                    const std::string& building)
{
    return building.empty() ? room : room + " · " + building;
}

// --- позиция пары в сетке недели (Р-32) -------------------------------------
// Строка 1 = 09:00–09:30, шаг 30 минут, 20 строк (09:00–19:00).
// --event-row = floor((мин − 540)/30) + 1, ограничение 1…20;
// --event-span = max(1, round(длительность/30)), не дальше 20-й строки.
// Пара вне окна прижимается к краю, время в карточке остаётся настоящим.
struct GridPos
{
    int row = 1;
    int span = 3;
};

inline GridPos gridPos(const Json::Value& e)
{
    GridPos p;
    const int start = minutesOf(str(e, "starts_at"));
    const int end = minutesOf(str(e, "ends_at"));
    if (start < 0)
        return p;
    const int offset = start - 540;
    const int rawRow = (offset >= 0 ? offset / 30 : -((-offset + 29) / 30)) + 1;
    p.row = std::clamp(rawRow, 1, 20);
    int span = 3;
    if (end > start)
        span = std::max(1, (end - start + 15) / 30);
    if (rawRow < 1)  // начало раньше 09:00 — видимая часть до конца пары
        span = std::max(1, span + rawRow - 1);
    p.span = std::clamp(span, 1, 21 - p.row);
    return p;
}

// --- карточка пары (эталон: div.lesson > .lesson-time + div + a.lesson-room) -
// data-* — для диалога занятия (static/js/app.js читает dataset). withGrid —
// style="--event-row/--event-span" для сетки недели.
inline std::string lessonCard(const Ctx& c, const Json::Value& e,
                              bool withGrid)
{
    const std::string st = str(e, "status");
    const std::string start = str(e, "starts_at");
    const std::string ends = str(e, "ends_at");
    const std::string subject = str(e, "discipline");
    const std::string room = str(e, "room");
    const std::string building = str(e, "building");
    std::string cls = "lesson";
    if (st == "moved" || st == "cancelled" || st == "substituted")
        cls += " " + st;
    std::string a = " data-event";
    if (e["id"].isIntegral())
        a += " data-event-id=\"" + std::to_string(e["id"].asInt64()) + "\"";
    a += " data-date=\"" + esc(dateOf(start)) + "\"";
    a += " data-date-label=\"" + esc(dayTitle(c, dateOf(start))) + "\"";
    a += " data-start=\"" + esc(timeOf(start)) + "\"";
    a += " data-end=\"" + esc(timeOf(ends)) + "\"";
    a += " data-kind=\"" + esc(str(e, "kind")) + "\"";
    a += " data-status=\"" + esc(st) + "\"";
    a += " data-room=\"" + esc(room) + "\"";
    if (!building.empty())
        a += " data-building=\"" + esc(building) + "\"";
    const std::string prevRoom = str(e, "prev_room");
    if (!prevRoom.empty())
        a += " data-prev-room=\"" + esc(prevRoom) + "\"";
    const std::string prevBuilding = str(e, "prev_building");
    if (!prevBuilding.empty())
        a += " data-prev-building=\"" + esc(prevBuilding) + "\"";
    const std::string prevStart = str(e, "prev_starts_at");
    if (!prevStart.empty())
    {
        a += " data-prev-start=\"" + esc(timeOf(prevStart)) + "\"";
        if (dateOf(prevStart) != dateOf(start))
            a += " data-prev-date-label=\"" +
                 esc(dayTitle(c, dateOf(prevStart))) + "\"";
    }
    if (e["teacher_id"].isIntegral())
        a += " data-teacher-id=\"" +
             std::to_string(e["teacher_id"].asInt64()) + "\"";
    if (withGrid)
    {
        const GridPos p = gridPos(e);
        a += " style=\"--event-row:" + std::to_string(p.row) +
             ";--event-span:" + std::to_string(p.span) + "\"";
    }

    std::string out = "<div class=\"" + cls + "\"" + a + ">";
    out += "<div class=\"lesson-time\"><strong>" + esc(timeOf(start)) +
           "</strong>";
    if (timeOf(ends) != "--:--")
        out += "<span>" + esc(timeOf(ends)) + "</span>";
    out += "</div><div><div class=\"lesson-subject\"><button type=\"button\" "
           "class=\"lesson-open\" data-lesson-open aria-label=\"" +
           esc(c.t("a11y.lessonOpen") + " " + subject) + "\">" + esc(subject) +
           "</button></div>";
    out += "<p class=\"lesson-detail\">" + esc(kindLabel(c, str(e, "kind")));
    if (st == "moved")
        out += " <span class=\"event-change\">· " + c.t("schedule.sheetChange") +
               "</span>";
    else if (st == "cancelled")
        out += " <span class=\"event-change\">· " + c.t("lesson.cancelled") +
               "</span>";
    else if (st == "substituted")
        out += " <span class=\"event-change\">· " + c.t("lesson.substituted") +
               "</span>";
    out += "</p></div>";
    if (st == "cancelled")
    {
        out += "<span class=\"lesson-room\"><s>" + esc(room) + "</s>";
        if (!building.empty())
            out += "<small>" + esc(building) + "</small>";
        out += "</span>";
    }
    else
    {
        out += "<a class=\"lesson-room\" href=\"" + mapHref(room) + "\">" +
               esc(room);
        if (!building.empty())
            out += "<small>" + esc(building) + "</small>";
        out += "</a>";
    }
    out += "</div>";
    return out;
}

// --- блок изменения (эталон .change-block[data-change-block]) --------------
// «было → стало» из полей события; «Подробнее» открывает диалог этой пары
// (отдельной страницы изменения в проде нет — таблица шага 0, строка 9).
// withDay — в заголовке день недели (блоки под сеткой недели).
inline std::string changeBlock(const Ctx& c, const Json::Value& e, bool withDay)
{
    const std::string st = str(e, "status");
    if (st != "moved" && st != "cancelled" && st != "substituted")
        return "";
    const std::string cur = str(e, "starts_at");
    const std::string prev = str(e, "prev_starts_at");
    const std::string ends = str(e, "ends_at");
    const std::string room = str(e, "room");
    const std::string building = str(e, "building");
    std::string out = "<div class=\"change-block\" data-change-block><h3>";
    if (withDay)
        out += esc(dayTitle(c, dateOf(cur), true)) + " · ";
    out += esc(timeOf(cur)) + " · " + esc(str(e, "discipline")) + "</h3>";
    const std::string span =
        esc(timeOf(cur)) + "–" + esc(timeOf(ends));
    if (st == "moved")
    {
        const bool sameTime = prev.empty() || (dateOf(prev) == dateOf(cur) &&
                                               timeOf(prev) == timeOf(cur));
        const std::string prevRoom = str(e, "prev_room");
        out += "<span class=\"change-label\">" +
               (sameTime ? c.t("schedule.changeRoomMoved")
                         : c.t("schedule.changeClassMoved")) +
               "</span>";
        out += "<div class=\"change-route\">";
        if (!prevRoom.empty() && prevRoom != room)
        {
            out += "<s>" +
                   esc(roomWithBuilding(prevRoom, str(e, "prev_building"))) +
                   "</s><span aria-hidden=\"true\">→</span>";
        }
        out += "<a data-change-room href=\"" + mapHref(room) + "\">" +
               esc(roomWithBuilding(room, building)) + "</a></div>";
        if (sameTime)
            out += "<p class=\"caption\">" + c.t("schedule.changeSameTime") +
                   " · " + span + "</p>";
        else
            out += "<p class=\"caption\">" + c.t("schedule.changeWas") + " " +
                   esc(dayTitle(c, dateOf(prev), true)) + ", " +
                   esc(timeOf(prev)) + "</p>";
    }
    else if (st == "cancelled")
    {
        out += "<span class=\"change-label\">" + c.t("schedule.changeCancelled") +
               "</span>";
        out += "<div class=\"change-route\"><s>" +
               esc(roomWithBuilding(room, building)) + "</s></div>";
        out += "<p class=\"caption\">" + span + "</p>";
    }
    else
    {
        out += "<span class=\"change-label\">" + c.t("schedule.changeSub") +
               "</span>";
        out += "<p class=\"caption\">" + span + "</p>";
    }
    if (e["id"].isIntegral())
        out += "<button type=\"button\" class=\"change-detail\" "
               "data-open-event=\"" +
               std::to_string(e["id"].asInt64()) + "\">" +
               (st == "moved" ? c.t("schedule.changeMore")
                              : c.t("schedule.changeMoreAny")) +
               "</button>";
    out += "</div>";
    return out;
}

}  // namespace para::schedview
