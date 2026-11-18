#include "controllers/ScheduleCtrl.h"

#include "utils/I18nView.h"

#include <string>

#include "services/ScheduleCache.h"
#include "services/ScheduleService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (02 §5.1: 500 —
// неожиданное, без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                ScheduleService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

// --- Страничная плоскость (Э7: GET /schedule, GET /schedule/segment) -----

// Разобранный query страничных роутов.
struct SegRequest
{
    std::string segment;  // «today» | «week»
    int offset = 0;       // недели, ± (02 §5.2), окно [-4..4]
    std::string date;     // день deep_link (/schedule?date=…, канон Э6/Р-17)
};

// Строгая календарность YYYY-MM-DD (2026-02-30 → false) — как validDate
// в RoomsService (02 §5.2, Э5а); локальная копия без <ctime>: длина/позиции
// разделителей, месяц 1..12, день по числу дней месяца с високосным годом.
bool validYmd(const std::string& s)
{
    if (s.size() != 10 || s[4] != '-' || s[7] != '-')
        return false;
    for (const int i : {0, 1, 2, 3, 5, 6, 8, 9})
        if (s[i] < '0' || s[i] > '9')
            return false;
    const int y = std::stoi(s.substr(0, 4));
    const int m = std::stoi(s.substr(5, 2));
    const int d = std::stoi(s.substr(8, 2));
    if (m < 1 || m > 12)
        return false;
    static const int kMonthDays[] = {31, 28, 31, 30, 31, 30,
                                     31, 31, 30, 31, 30, 31};
    const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    return d >= 1 &&
           d <= kMonthDays[m - 1] + static_cast<int>(m == 2 && leap);
}

// segment — today|week (по умолчанию today); offset — целое [-4..4]
// (по умолчанию 0); date — календарная дата deep_link (пусто → сегмент
// «Сегодня» показывает сегодня UTC). Нечисловой offset / чужое имя
// сегмента / некалендарная date — false → 400 bad_request (02 §5.1;
// невалидная query-дата — как в /v1/rooms/{id}/slots, 02 §5.2). Паттерн
// разбора offset — как в getWeek (getOptionalParameter<int> молчит на
// не-числе).
bool parseSegmentQuery(const HttpRequestPtr& req, SegRequest& out)
{
    out.segment = req->getParameter("segment");
    if (out.segment.empty())
        out.segment = "today";
    if (out.segment != "today" && out.segment != "week")
        return false;
    out.offset = 0;
    const auto parsed = req->getOptionalParameter<int>("offset");
    if (parsed)
        out.offset = *parsed;
    else if (!req->getParameter("offset").empty())
        return false;
    out.date.clear();
    const std::string dateParam = req->getParameter("date");
    if (!dateParam.empty())
    {
        if (!validYmd(dateParam))
            return false;
        out.date = dateParam;
    }
    return out.offset >= -4 && out.offset <= 4;
}

// 400 по 02 §5.1: API-плоскости — JSON {error:{code,message}}; htmx-запросу —
// HTML-фрагмент (02 §5.1: «Ошибки валидации/бизнес-правил приходят
// HTML-фрагментом (для htmx)»; гайд FE §5). htmx по умолчанию не меняет DOM
// на 4xx — текст показывает error-box app.js (03 §5.3); фрагмент —
// контрактная форма ответа (Р-19 в docs/decisions.md).
HttpResponsePtr badSegmentRequest(bool htmx)
{
    const std::string message =
        "segment — today|week; offset — целое число от -4 до 4; "
        "date — календарная дата YYYY-MM-DD";
    if (!htmx)
        return api::errorResponse("bad_request", message, k400BadRequest);
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k400BadRequest);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody("<div class=\"notice danger\" role=\"alert\">" +
                  HttpViewData::htmlTranslate(message) + "</div>");
    return resp;
}

HttpResponsePtr internalError()
{
    return api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError);
}

// Сериализация тел сервиса для view-данных (сырая кириллица — emitUTF8,
// как в api::jsonResponse с Э6; разбор — в партиале, вывод экранирует
// партиал — 02 §6.4).
std::string toJson(const Json::Value& body)
{
    Json::StreamWriterBuilder w;
    w["emitUTF8"] = true;
    return Json::writeString(w, body);
}

// Тело {"date","events"} на ПРОИЗВОЛЬНЫЙ день deep_link — сборка из тех же
// публичных методов ScheduleService, из которых собран ScheduleService::today
// (groupOfUser + eventsOnDay; 02 §5.2): сам сервис не правится (sql/ и
// src/services/ — BE-дорожка, AGENTS §5.8), SQL в контроллере нет (§5.5).
// staff (группы нет, Р-6) — пустой день; ошибка БД — fail-closed (500).
void dayBody(const drogon::orm::DbClientPtr& db,
             int64_t userId,
             const std::string& date,
             const std::function<void(ScheduleService::Result)>& onDone)
{
    ScheduleService::groupOfUser(
        db, userId,
        [db, date, onDone](ScheduleService::GroupLookup lookup,
                           std::optional<int64_t> groupId) {
            if (lookup == ScheduleService::GroupLookup::Error)
            {
                onDone(ScheduleService::Result{false, Json::Value()});
                return;
            }
            if (lookup == ScheduleService::GroupLookup::NotFound)
            {
                Json::Value body;
                body["date"] = date;
                body["events"] = Json::Value(Json::arrayValue);
                onDone(ScheduleService::Result{true, std::move(body)});
                return;
            }
            ScheduleService::eventsOnDay(
                db, *groupId, date,
                [date, onDone](bool ok, Json::Value events) {
                    if (!ok)
                    {
                        onDone(ScheduleService::Result{false, Json::Value()});
                        return;
                    }
                    Json::Value body;
                    body["date"] = date;
                    body["events"] = std::move(events);
                    onDone(ScheduleService::Result{true, std::move(body)});
                });
        });
}

// Понедельник недели, содержащей ymd, — в днях от эпохи (для сдвига недели
// к дате deep_link; арифметика civil-дат, без <ctime>).
long civilDays(const std::string& ymd)
{
    if (ymd.size() < 10)
        return 0;
    const int y = std::stoi(ymd.substr(0, 4));
    const int m = std::stoi(ymd.substr(5, 2));
    const int d = std::stoi(ymd.substr(8, 2));
    const long yy = y - (m <= 2);
    const long era = (yy >= 0 ? yy : yy - 399) / 400;
    const long yoe = yy - era * 400;
    const long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097L + doe - 719468L;
}

// Данные сегмента — ТЕ ЖЕ методы ScheduleService, что стоят за JSON API
// (day/week/changeList, 02 §5.2): SQL в контроллере нет (AGENTS §5.5).
// «Сегодня»: без date — канонический ScheduleService::today (UTC-сейчас);
// с date (deep_link /schedule?date=…, канон Э6 и тапа по переносу Р-17) —
// dayBody на эту дату. «Неделя»: offset из query, а при наличии date —
// неделя, СОДЕРЖАЩАЯ дату (разница понедельников; date приоритетнее offset).
// Неделя = week + changeList(days=7) последовательно; оба колбэка в каждой
// ветке копируются (урок Э2 шаг 1: move в одной ветке оставил бы второму
// пустой std::function → bad_function_call на error-пути БД).
void fetchSegment(const SegRequest& q,
                  int64_t userId,
                  const std::string& today,
                  const std::function<void(const Json::Value& body,
                                           const Json::Value& changes)>&
                      onDone,
                  const std::function<void(const HttpResponsePtr&)>& onError)
{
    const auto db = app().getDbClient("para");
    if (q.segment == "today")
    {
        if (q.date.empty())
        {
            ScheduleService::today(
                db, userId, today,
                [onDone, onError](ScheduleService::Result r) {
                    if (!r.ok)
                    {
                        onError(internalError());
                        return;
                    }
                    static const Json::Value kNoChanges(Json::arrayValue);
                    onDone(r.body, kNoChanges);
                });
            return;
        }
        dayBody(db, userId, q.date,
                [onDone, onError](ScheduleService::Result r) {
                    if (!r.ok)
                    {
                        onError(internalError());
                        return;
                    }
                    static const Json::Value kNoChanges(Json::arrayValue);
                    onDone(r.body, kNoChanges);
                });
        return;
    }
    int offset = q.offset;
    if (!q.date.empty())
    {
        const std::string dateMonday = ScheduleService::weekStart(q.date, 0);
        const std::string todayMonday = ScheduleService::weekStart(today, 0);
        offset = static_cast<int>((civilDays(dateMonday) -
                                   civilDays(todayMonday)) /
                                  7);
    }
    ScheduleService::week(
        db, userId, offset, &ScheduleCache::instance(), today,
        [db, userId, today, onDone,
         onError](ScheduleService::Result r) {
            if (!r.ok)
            {
                onError(internalError());
                return;
            }
            // Копия тела недели — внутренняя лямбда переживает внешний
            // колбэк (см. комментарий ScheduleService::week).
            const Json::Value weekBody = r.body;
            ScheduleService::changeList(
                db, userId, 7, today,
                [weekBody, onDone, onError](ScheduleService::Result r2) {
                    if (!r2.ok)
                    {
                        onError(internalError());
                        return;
                    }
                    onDone(weekBody, r2.body);
                });
        });
}

}  // namespace

void ScheduleCtrl::getToday(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    ScheduleService::today(
        app().getDbClient("para"), session.userId, ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ScheduleCtrl::getWeek(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // offset — целое недель, ± (02 §5.2); отсутствует → 0; не число → 400.
    int offset = 0;
    const auto parsed = req->getOptionalParameter<int>("offset");
    if (parsed)
    {
        offset = *parsed;
    }
    else if (!req->getParameter("offset").empty())
    {
        cb(api::errorResponse("bad_request",
                              "offset — ожидается целое число (недель, ±)",
                              k400BadRequest));
        return;
    }
    ScheduleService::week(
        app().getDbClient("para"), session.userId, offset,
        &ScheduleCache::instance(), ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ScheduleCtrl::getChanges(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // days — целое 1..31 (02 §5.2: query days=7 по умолчанию).
    int days = 7;
    const auto parsed = req->getOptionalParameter<int>("days");
    if (parsed)
    {
        days = *parsed;
    }
    else if (!req->getParameter("days").empty())
    {
        cb(api::errorResponse("bad_request", "days — ожидается целое число",
                              k400BadRequest));
        return;
    }
    if (days < 1 || days > 31)
    {
        cb(api::errorResponse("bad_request", "days — ожидается целое 1..31",
                              k400BadRequest));
        return;
    }
    ScheduleService::changeList(
        app().getDbClient("para"), session.userId, days,
        ScheduleService::todayUtc(),
        [cb = std::move(cb)](ScheduleService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

// --- Страничная плоскость (Э7; см. шапку заголовка) -----------------------

void ScheduleCtrl::page(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница) — user_id только
    // из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    SegRequest q;
    if (!parseSegmentQuery(req, q))
    {
        cb(badSegmentRequest(false));
        return;
    }
    const std::string today = ScheduleService::todayUtc();
    // В view-данные — только сериализованные тела сервиса и серверные
    // константы; строки события экранирует партиал при выводе (02 §6.4).
    // todayIso — день сегмента «Сегодня» (дата deep_link или сегодня UTC);
    // для «Недели» — всегда сегодня UTC (бейдж «Сегодня» у реального дня).
    const std::string dayIso =
        q.segment == "week" ? today : (q.date.empty() ? today : q.date);
    fetchSegment(
        q, session.userId, today,
        [cb, q, dayIso, realToday = today,
         lang = i18n::resolveRequestLang(req)](
            const Json::Value& body,
            const Json::Value& changes) mutable {
            // 5.18 (шаг 3): строки словаря — в view-данные ДО рендера
            // (Р-16); язык — para_lang → Accept-Language → ru.
            HttpViewData data;
            i18n::insertUiStrings(data, lang);
            data.insert("pageTitle", i18n::pageTitleOf("nav.schedule", lang));
            data.insert("activeTab", std::string("schedule"));
            data.insert("segment", q.segment);
            data.insert("todayIso", dayIso);
            // Реальное «сегодня» (UTC проекта) отдельно от показанного дня:
            // aria-current="date" полосы недели — выбранный день (todayIso),
            // точка «сегодня» и колонка «Сегодня» — реальный день (v3, шаг 3).
            data.insert("realTodayIso", realToday);
            data.insert("bodyJson", toJson(body));
            data.insert("changesJson", toJson(changes));
            cb(HttpResponse::newHttpViewResponse("schedule",
                                                 std::move(data)));
        },
        cb);
}

void ScheduleCtrl::segment(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Фрагменты — только по htmx (02 §3.2: HX-Request); прямой заход — на
    // страницу (браузер сам вернётся к сегменту через query страницы).
    const bool htmx = req->getHeader("HX-Request") == "true";
    if (!htmx)
    {
        cb(HttpResponse::newRedirectionResponse("/schedule", k303SeeOther));
        return;
    }
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    SegRequest q;
    if (!parseSegmentQuery(req, q))
    {
        cb(badSegmentRequest(true));
        return;
    }
    const std::string today = ScheduleService::todayUtc();
    const std::string dayIso =
        q.segment == "week" ? today : (q.date.empty() ? today : q.date);
    fetchSegment(
        q, session.userId, today,
        [cb, q, dayIso, realToday = today,
         lang = i18n::resolveRequestLang(req)](
            const Json::Value& body,
            const Json::Value& changes) mutable {
            // 5.18 (шаг 3): htmx-фрагмент несёт те же словарные строки
            // (сегментные кнопки/подписи дней) — бандл в view-данные
            // ДО рендера (Р-16).
            HttpViewData data;
            i18n::insertUiStrings(data, lang);
            data.insert("segment", q.segment);
            data.insert("todayIso", dayIso);
            // Реальное «сегодня» (UTC проекта) отдельно от показанного дня:
            // aria-current="date" полосы недели — выбранный день (todayIso),
            // точка «сегодня» и колонка «Сегодня» — реальный день (v3, шаг 3).
            data.insert("realTodayIso", realToday);
            data.insert("bodyJson", toJson(body));
            data.insert("changesJson", toJson(changes));
            // Имя шаблона — класс сгенерированного партиала (файл без
            // расширения; дефис в имени файла drogon_ctl в класс не
            // допускает — schedule_today, не schedule-today).
            cb(HttpResponse::newHttpViewResponse(
                q.segment == "week" ? "schedule_week" : "schedule_today",
                std::move(data)));
        },
        cb);
}
