#include "controllers/NotificationsFeedCtrl.h"

#include "utils/I18nView.h"
#include "utils/StaffRouting.h"

#include "services/NotificationsFeedService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Вердикт guard'а → ответ (02 §6.3/§5.1): чужое существующее — 403
// forbidden (дословный пример 02 §5.1) ДО пометки, несуществующее — 404
// not_found, ошибка БД — 500 (fail-closed). Own обрабатывает вызывающий.
bool sendVerdict(std::function<void(const HttpResponsePtr&)>& cb,
                 OwnershipGuard::Verdict v)
{
    switch (v)
    {
        case OwnershipGuard::Verdict::Own:
            return false;  // владение подтверждено — продолжить
        case OwnershipGuard::Verdict::NotFound:
            cb(api::errorResponse("not_found", "Уведомление не найдено",
                                  k404NotFound));
            return true;
        case OwnershipGuard::Verdict::Forbidden:
            cb(api::errorResponse("forbidden", "Доступ только к своим данным",
                                  k403Forbidden));
            return true;
        case OwnershipGuard::Verdict::Error:
            cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                  k500InternalServerError));
            return true;
    }
    return true;  // недостижимо при полном switch над enum
}

// 204 — пустое тело без Content-Type JSON (как DELETE брони в RoomsCtrl).
HttpResponsePtr noContent()
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k204NoContent);
    return resp;
}

}  // namespace

void NotificationsFeedCtrl::feed(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");

    // query 02 §5.2: unread=1 / type / cursor. Нечисловой cursor → 400
    // (стиль days в ScheduleCtrl); иное значение unread → 400 (стиль
    // upcoming в RoomsCtrl); словарь type и существование курсора — сервис.
    NotificationsFeedService::Query q;
    const std::string rawUnread = req->getParameter("unread");
    if (rawUnread == "1")
        q.unreadOnly = true;
    else if (!rawUnread.empty())
    {
        cb(api::errorResponse("bad_request", "unread — ожидается 1",
                              k400BadRequest));
        return;
    }
    const std::string rawType = req->getParameter("type");
    if (!rawType.empty())
        q.type = rawType;
    const std::string rawCursor = req->getParameter("cursor");
    if (!rawCursor.empty())
    {
        try
        {
            q.cursorId = std::stoll(rawCursor);
            if (*q.cursorId <= 0)
                throw std::invalid_argument("cursor");
        }
        catch (const std::exception&)
        {
            cb(api::errorResponse("bad_request",
                                  "cursor — ожидается положительное число",
                                  k400BadRequest));
            return;
        }
    }

    NotificationsFeedService::feed(
        app().getDbClient("para"), session.userId, q,
        [cb = std::move(cb)](NotificationsFeedService::FeedResult r) mutable {
            switch (r.status)
            {
                case NotificationsFeedService::FeedStatus::Ok:
                    cb(api::jsonResponse(std::move(r.body)));  // UTF-8 (гейт р1)
                    return;
                case NotificationsFeedService::FeedStatus::BadRequest:
                    cb(api::errorResponse("bad_request", r.error,
                                          k400BadRequest));
                    return;
                case NotificationsFeedService::FeedStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

void NotificationsFeedCtrl::markRead(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t notificationId)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Правило код-ревью (OwnershipGuard.h): path-id → guard ДО любой отдачи
    // данных и ДО пометки (чужое существующее → 403, уведомление не
    // изменяется). cb внешней лямбды — копия (стреляет один раз),
    // Own-ветка move'ит её дальше — как в GradesCtrl::getGrade.
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Notifications, session.userId,
        notificationId,
        [db, session, notificationId,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            NotificationsFeedService::markRead(
                db, session.userId, notificationId,
                [cb = std::move(cb)](
                    NotificationsFeedService::ReadStatus s) mutable {
                    // Ok — помечено; AlreadyRead — было прочитано ранее
                    // (идемпотентно: тот же 204, без ошибки и без дубля
                    // метки — условный UPDATE read_at IS NULL не трогает
                    // существующую метку). Error → 500.
                    if (s == NotificationsFeedService::ReadStatus::Error)
                        cb(api::errorResponse(
                            "internal", "Внутренняя ошибка сервера",
                            k500InternalServerError));
                    else
                        cb(noContent());
                });
        });
}

void NotificationsFeedCtrl::markAllRead(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    NotificationsFeedService::markAllRead(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](
            NotificationsFeedService::MarkAllResult r) mutable {
            // marked=0 («всё уже прочитано») — тот же 204 (идемпотентно).
            if (r.ok)
                cb(noContent());
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}

// --- Страничная плоскость (Э7, экран «Уведомления»; см. шапку заголовка) --

namespace {

// 5.17: ?segment= — notifications (по умолчанию) | news | events; иное
// непустое значение → false → 400 bad_request (паттерн parseSegmentQuery
// ScheduleCtrl: чужое имя сегмента не молчит пустой страницей).
bool parseSegment(const HttpRequestPtr& req, std::string& out)
{
    out = req->getParameter("segment");
    if (out.empty())
        out = "notifications";
    return out == "notifications" || out == "news" || out == "events" ||
           out == "announcements";  // фиксы 2026-11-19: лента объявлений
}

// 400 по 02 §5.1: страница — JSON {error:{code,message}}; htmx-запросу —
// HTML-фрагмент (02 §5.1, Р-19; копия badSegmentRequest ScheduleCtrl).
HttpResponsePtr badSegment(bool htmx)
{
    const std::string message =
        "segment — notifications|news|events|announcements";
    if (!htmx)
        return api::errorResponse("bad_request", message, k400BadRequest);
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k400BadRequest);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody("<div class=\"notice danger\" role=\"alert\">" +
                  HttpViewData::htmlTranslate(message) + "</div>");
    return resp;
}

}  // namespace

void NotificationsFeedCtrl::page(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница); данных страница
    // не содержит — view-данные только каркасные + выбранный сегмент 5.17.
    std::string segment;
    if (!parseSegment(req, segment))
    {
        cb(badSegment(false));
        return;
    }
    // 5.18 (шаг 3): строки словаря — в view-данные ДО рендера (Р-16);
    // язык — para_lang → Accept-Language → ru.
    HttpViewData data;
    const auto lang = i18n::resolveRequestLang(req);
    i18n::insertUiStrings(data, lang);
    para::insertStaffFlag(data, req);  // навигация сотрудника (Р-35)
    data.insert("pageTitle", i18n::pageTitleOf("notif.title", lang));
    // Экран НЕ таб (03 §3.2: пять табов, колокольчик — в шапке): пустой
    // activeTab → ни один пункт таб-бара не получает aria-current.
    // Фиксы 2026-11-19: у сотрудника «Новости»/«Объявления» — пункты его
    // навигации (topbar), активный подсвечивается по сегменту.
    data.insert("activeTab", (segment == "news" || segment == "announcements")
                                 ? segment
                                 : std::string(""));
    // Сегмент «Уведомления | Новости | События» (5.17): партиал читает и
    // рисует переключатель с нужным aria-pressed; данные — клиент (шапка
    // NotificationsFeedCtrl.h).
    data.insert("segment", segment);
    cb(HttpResponse::newHttpViewResponse("notifications", std::move(data)));
}

void NotificationsFeedCtrl::segment(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Фрагменты — только по htmx (02 §3.2: HX-Request); прямой заход — на
    // страницу (браузер сам вернётся к сегменту через query страницы).
    const bool htmx = req->getHeader("HX-Request") == "true";
    if (!htmx)
    {
        cb(HttpResponse::newRedirectionResponse("/notifications",
                                                k303SeeOther));
        return;
    }
    std::string segment;
    if (!parseSegment(req, segment))
    {
        cb(badSegment(true));
        return;
    }
    // 5.18 (шаг 3): htmx-фрагмент — те же словарные строки страницы
    // (кнопки сегментов/чипы/пустые состояния), бандл ДО рендера (Р-16).
    HttpViewData data;
    i18n::insertUiStrings(data, i18n::resolveRequestLang(req));
    data.insert("segment", segment);
    // Партиал-оболочка (разметка состояний — данные грузит клиент из
    // /v1/notifications и /v1/news; имя шаблона — файл без расширения).
    cb(HttpResponse::newHttpViewResponse("notifications_segment",
                                         std::move(data)));
}
