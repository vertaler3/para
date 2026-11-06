#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Бронь аудитории (5.10; Э5а шаги 1–2; 02 §5.2). Тонкий контроллер
// (AGENTS.md §5.5): SQL, сетка слотов и правила бронирования —
// RoomsService; роуты под LoginFilter — без cookie 401 от фильтра
// (02 §6.3). user_id — только из сессии.
//
//   GET /v1/rooms                — каталог аудиторий (path-id нет);
//   GET /v1/rooms/{id}/slots     — сетка слотов дня: свободно/занято/ваша
//       бронь (?date=YYYY-MM-DD, отсутствие — сегодня UTC); несуществующая
//       аудитория → 404 not_found; невалидная дата → 400 bad_request;
//   POST /v1/rooms/{id}/bookings — забронировать интервал (body: starts_at,
//       ends_at — 02 §5.2); двойная бронь того же слота → 400, пересечение
//       → 409 («только что заняли»), невалидное тело → 400, комнаты нет →
//       404, создано → 201; исключение БД (UNIQUE-гонка) → 400, не 500
//       (RoomsService::create);
//   GET /v1/room-bookings        — мои активные брони (?upcoming=1 —
//       только предстоящие);
//   DELETE /v1/room-bookings/{id} — отмена своей активной брони → 204;
//       ПЕРВЫЙ path-id личного объекта шага — OwnershipGuard::check
//       (Table::RoomBookings) ДО любых данных (правило код-ревью
//       OwnershipGuard.h; шаг 8 автотеста 02 §9.3 / 01 §5.10 «чужая бронь
//       по подставленному адресу → 403»): Own → 204-цепочка, NotFound →
//       404, Forbidden → 403 «Доступ только к своим данным» (дословный
//       пример 02 §5.1), Error → 500 fail-closed. Повторный DELETE своей
//       уже отменённой → 409 invalid_status_transition (не booked).
//
// Ошибки — единый формат 02 §5.1. Guard НЕ вызывается на /v1/rooms и
// /v1/rooms/{id}/...: аудитория — общий ресурс каталога (её id чужим не
// бывает, «занято» чужой бронью читается всеми по смыслу 5.10); личный
// объект здесь — бронь (room_bookings/{id}), его охраняет guard выше.
// 403 контракта POST bookings (02 §5.2) возникает на DELETE чужой брони.
//
// query date/capacity каталога (02 §5.2) — не в шаге 1: постановка —
// «каталог комнат»; фильтры занятости требуют полей бронирования шага 2.
class RoomsCtrl : public HttpController<RoomsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RoomsCtrl::getRooms, "/v1/rooms", Get, "LoginFilter");
    ADD_METHOD_TO(RoomsCtrl::getSlots, "/v1/rooms/{id}/slots", Get,
                  "LoginFilter");
    ADD_METHOD_TO(RoomsCtrl::createBooking, "/v1/rooms/{id}/bookings", Post,
                  "LoginFilter");
    ADD_METHOD_TO(RoomsCtrl::myBookings, "/v1/room-bookings", Get,
                  "LoginFilter");
    ADD_METHOD_TO(RoomsCtrl::cancelBooking, "/v1/room-bookings/{id}", Delete,
                  "LoginFilter");
    METHOD_LIST_END

    void getRooms(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void getSlots(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t roomId);
    void createBooking(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb,
                       int64_t roomId);
    void myBookings(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
    void cancelBooking(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& cb,
                       int64_t bookingId);
};
