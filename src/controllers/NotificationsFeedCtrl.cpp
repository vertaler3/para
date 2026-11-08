#include "controllers/NotificationsFeedCtrl.h"

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

void NotificationsFeedCtrl::page(
    const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& cb)
{
    (void)req;  // сессию проверил LoginFilter (303 без неё — страница);
                // данных страница не содержит — view-данные только каркасные
    HttpViewData data;
    data.insert("pageTitle", std::string("Уведомления · Пара"));
    // Экран НЕ таб (03 §3.2: пять табов, колокольчик — в шапке): пустой
    // activeTab → ни один пункт таб-бара не получает aria-current.
    data.insert("activeTab", std::string(""));
    cb(HttpResponse::newHttpViewResponse("notifications", std::move(data)));
}
