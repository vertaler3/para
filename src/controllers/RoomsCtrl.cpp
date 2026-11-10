#include "controllers/RoomsCtrl.h"

#include "services/OwnershipGuard.h"
#include "services/RoomsService.h"
#include "services/SessionService.h"  // utcNow — фильтр upcoming
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (500 — неожиданное,
// без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                RoomsService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

// Вердикт guard'а → ответ (02 §6.3/§5.1): чужое — 403 forbidden (дословный
// пример 02 §5.1), не существующее — 404 not_found, ошибка БД — 500
// (fail-closed). Own обрабатывает вызывающий (там нужен выборка/сервис).
bool sendVerdict(std::function<void(const HttpResponsePtr&)>& cb,
                 OwnershipGuard::Verdict v)
{
    switch (v)
    {
        case OwnershipGuard::Verdict::Own:
            return false;  // владение подтверждено — продолжить
        case OwnershipGuard::Verdict::NotFound:
            cb(api::errorResponse("not_found", "Запись не найдена",
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

}  // namespace

void RoomsCtrl::getRooms(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — доступ только по cookie (02 §6.3).
    // query контракта 02 §5.2 (правки по гейту Э5а, раунд 1):
    //   ?capacity=N — фильтр «вместимость не меньше» (найти аудиторию на
    //     N человек); отсутствует → без фильтра; не целое → 400 (стиль
    //     days/offset в ScheduleCtrl);
    //   ?date=YYYY-MM-DD — принимается и валидируется (невалидная → 400,
    //     единый стиль ?date= сетки); на выдачу каталога не влияет — поля
    //     занятости дня в контракте каталога 02 §5.2 не описаны (занятость
    //     дня — /v1/rooms/{id}/slots?date=).
    std::optional<int64_t> minCapacity;
    const auto parsed = req->getOptionalParameter<int64_t>("capacity");
    if (parsed)
    {
        minCapacity = *parsed;
    }
    else if (!req->getParameter("capacity").empty())
    {
        cb(api::errorResponse("bad_request",
                              "capacity — ожидается целое число",
                              k400BadRequest));
        return;
    }
    const std::string rawDate = req->getParameter("date");
    if (!rawDate.empty() && !RoomsService::validDate(rawDate))
    {
        cb(api::errorResponse("bad_request",
                              "date — ожидается дата YYYY-MM-DD",
                              k400BadRequest));
        return;
    }
    RoomsService::catalog(
        app().getDbClient("para"), minCapacity,
        [cb = std::move(cb)](RoomsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void RoomsCtrl::getSlots(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb,
                         int64_t roomId)
{    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3):
    // «ваша бронь» в сетке — по нему (RoomsService::slots).
    const auto session = req->attributes()->get<para::SessionInfo>("session");

    // ?date= — YYYY-MM-DD (02 §5.2 «query: date»); отсутствует → сегодня
    // UTC (как /v1/schedule/today); невалидный/некалендарный → 400
    // bad_request (02 §5.1), сетку по такой дате не строим.
    const std::string rawDate = req->getParameter("date");
    const std::string date =
        rawDate.empty() ? RoomsService::todayUtc() : rawDate;
    if (!RoomsService::validDate(date))
    {
        cb(api::errorResponse("bad_request",
                              "date — ожидается дата YYYY-MM-DD",
                              k400BadRequest));
        return;
    }

    RoomsService::slots(
        app().getDbClient("para"), roomId, session.userId, date,
        [cb = std::move(cb)](RoomsService::SlotsResult r) mutable {
            switch (r.status)
            {
                case RoomsService::SlotsStatus::Ok:
                    cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
                    return;
                case RoomsService::SlotsStatus::NotFound:
                    cb(api::errorResponse("not_found", "Аудитория не найдена",
                                          k404NotFound));
                    return;
                case RoomsService::SlotsStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

void RoomsCtrl::createBooking(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb,
                              int64_t roomId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // body: starts_at, ends_at (02 §5.2). Формат/интервал валидирует
    // сервис (InvalidBody → 400); здесь — только наличие JSON-тела и строк.
    const auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("starts_at") ||
        !json->isMember("ends_at") || !(*json)["starts_at"].isString() ||
        !(*json)["ends_at"].isString())
    {
        cb(api::errorResponse(
            "bad_request",
            "Ожидается JSON-тело со строками starts_at и ends_at",
            k400BadRequest));
        return;
    }
    RoomsService::create(
        app().getDbClient("para"),
        roomId,
        session.userId,
        (*json)["starts_at"].asString(),
        (*json)["ends_at"].asString(),
        [cb = std::move(cb)](RoomsService::CreateResult r) mutable {
            switch (r.status)
            {
                case RoomsService::CreateStatus::Created:
                {
                    auto resp =
                        HttpResponse::newHttpJsonResponse(std::move(r.body));
                    resp->setStatusCode(k201Created);
                    cb(std::move(resp));
                    return;
                }
                case RoomsService::CreateStatus::SlotTaken:
                    // «Двойная бронь» того же слота — серверная проверка
                    // или UNIQUE idx_bookings_active в гонке (гайд BE §8:
                    // повторная бронь того же слота другим клиентом → 400).
                    cb(api::errorResponse("slot_taken", "Слот уже занят",
                                          k400BadRequest));
                    return;
                case RoomsService::CreateStatus::Conflict:
                    // Пересечение интервалов — «конфликт, только что
                    // заняли» (случай FE-сетки, 03 §4.9.5).
                    cb(api::errorResponse(
                        "conflict",
                        "Интервал пересекается с активной бронью",
                        k409Conflict));
                    return;
                case RoomsService::CreateStatus::Blocked:
                    // Пересечение с блокировкой деканата (шаг 3): бронь
                    // поверх блокировки невозможна.
                    cb(api::errorResponse("blocked",
                                          "Интервал заблокирован деканатом",
                                          k409Conflict));
                    return;
                case RoomsService::CreateStatus::InvalidBody:
                    cb(api::errorResponse(
                        "bad_request",
                        "starts_at/ends_at — ожидается ISO 8601 UTC "
                        "(YYYY-MM-DDTHH:MM:SSZ), ends_at позже starts_at",
                        k400BadRequest));
                    return;
                case RoomsService::CreateStatus::NotFound:
                    cb(api::errorResponse("not_found", "Аудитория не найдена",
                                          k404NotFound));
                    return;
                case RoomsService::CreateStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

void RoomsCtrl::myBookings(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // ?upcoming=1 (02 §5.2) — только предстоящие; отсутствие — все активные;
    // иное значение → 400 (стиль overdue в StaffCtrl: известные значения —
    // только те, что описаны контрактом).
    bool upcoming = false;
    const std::string raw = req->getParameter("upcoming");
    if (raw == "1")
        upcoming = true;
    else if (!raw.empty())
    {
        cb(api::errorResponse("bad_request", "upcoming — ожидается 1",
                              k400BadRequest));
        return;
    }
    RoomsService::myBookings(
        app().getDbClient("para"), session.userId, upcoming,
        SessionService::utcNow(),
        [cb = std::move(cb)](RoomsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void RoomsCtrl::cancelBooking(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb,
                              int64_t bookingId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Правило код-ревью (OwnershipGuard.h): path-id ЛИЧНОГО объекта → guard
    // ДО любой отдачи данных (шаг 8 автотеста 02 §9.3: чужая бронь по
    // подставленному адресу → 403; 02 §6.3: NotFound/Forbidden различаются).
    // cb внешней лямбды — копия (стреляет один раз), Own-ветка move'ит её
    // дальше — как в GradesCtrl::getGrade.
    OwnershipGuard::check(
        db, OwnershipGuard::Table::RoomBookings, session.userId, bookingId,
        [db, bookingId, cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            RoomsService::cancelById(
                db, bookingId,
                [cb = std::move(cb)](RoomsService::CancelStatus s) mutable {
                    switch (s)
                    {
                        case RoomsService::CancelStatus::Canceled:
                        {
                            auto resp = HttpResponse::newHttpResponse();
                            resp->setStatusCode(k204NoContent);
                            cb(std::move(resp));
                            return;
                        }
                        case RoomsService::CancelStatus::NotActive:
                            // Повторная отмена / блокировка — статус не
                            // booked (в духе invalid_status_transition Э5).
                            cb(api::errorResponse(
                                "invalid_status_transition",
                                "Отменить можно только активную бронь",
                                k409Conflict));
                            return;
                        case RoomsService::CancelStatus::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Запись не найдена",
                                                  k404NotFound));
                            return;
                        case RoomsService::CancelStatus::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void RoomsCtrl::blockRoom(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb,
                          int64_t roomId)
{
    // Сессию и РОЛЬ проверили фильтры (LoginFilter, StaffFilter — 02 §6.3:
    // студент на /v1/staff/* → 403); staffUserId — только из сессии.
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // body: starts_at, ends_at, reason (02 §5.2). Формат интервала валидирует
    // сервис; здесь — наличие JSON, строки и непустой reason.
    const auto json = req->getJsonObject();
    if (json == nullptr || !json->isMember("starts_at") ||
        !json->isMember("ends_at") || !json->isMember("reason") ||
        !(*json)["starts_at"].isString() || !(*json)["ends_at"].isString() ||
        !(*json)["reason"].isString() ||
        (*json)["reason"].asString().empty())
    {
        cb(api::errorResponse(
            "bad_request",
            "Ожидается JSON-тело со строками starts_at, ends_at и reason",
            k400BadRequest));
        return;
    }
    RoomsService::block(
        app().getDbClient("para"),
        roomId,
        session.userId,
        (*json)["starts_at"].asString(),
        (*json)["ends_at"].asString(),
        (*json)["reason"].asString(),
        [cb = std::move(cb)](RoomsService::BlockResult r) mutable {
            switch (r.status)
            {
                case RoomsService::BlockStatus::Created:
                {
                    auto resp =
                        HttpResponse::newHttpJsonResponse(std::move(r.body));
                    resp->setStatusCode(k201Created);
                    cb(std::move(resp));
                    return;
                }
                case RoomsService::BlockStatus::InvalidBody:
                    cb(api::errorResponse(
                        "bad_request",
                        "starts_at/ends_at — ожидается ISO 8601 UTC "
                        "(YYYY-MM-DDTHH:MM:SSZ), ends_at позже starts_at",
                        k400BadRequest));
                    return;
                case RoomsService::BlockStatus::NotFound:
                    cb(api::errorResponse("not_found", "Аудитория не найдена",
                                          k404NotFound));
                    return;
                case RoomsService::BlockStatus::Error:
                    cb(api::errorResponse("internal",
                                          "Внутренняя ошибка сервера",
                                          k500InternalServerError));
                    return;
            }
        });
}

// --- Страничная плоскость (Э7: GET /rooms; см. шапку заголовка) -----------

void RoomsCtrl::roomsPage(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (страница без сессии → 303 /login —
    // правка фильтра Э7/Р-19, как ScheduleCtrl::page). Тонкая страница без
    // данных (AGENTS §5.5): каталог/сетку слотов/«Мои брони» грузит клиент
    // из тех же JSON-роутов этого контроллера (GET /v1/rooms,
    // GET /v1/rooms/{id}/slots, GET /v1/room-bookings — 02 §5.2) — паттерн
    // страницы «Новая заявка» (ApplicationsCtrl::newPage): значения не
    // дублируются сервером, состояния 03 §5 — в шаблоне и app.js.
    // query контракта каталога ?capacity= — тоже на клиенте (fetch).
    (void)req;
    HttpViewData data;
    data.insert("pageTitle", std::string("Аудитории · Пара"));
    // Экран НЕ таб (03 §3.1 — вход по ghost-ссылке «Забронировать
    // аудиторию» шапки расписания, 03 §4.3/§4.9.5): пустой activeTab —
    // ни один пункт таб-бара не получает aria-current.
    data.insert("activeTab", std::string(""));
    cb(HttpResponse::newHttpViewResponse("rooms", std::move(data)));
}
