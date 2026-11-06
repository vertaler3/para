#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Бронь аудитории (5.10; Э5а шаги 1–3; 02 §5.2). Тонкий контроллер
// (AGENTS.md §5.5): SQL, сетка слотов и правила бронирования/блокировки —
// RoomsService; роуты под LoginFilter — без cookie 401 от фильтра
// (02 §6.3). user_id — только из сессии.
//
//   GET /v1/rooms                — каталог аудиторий (path-id нет);
//   GET /v1/rooms/{id}/slots     — сетка слотов дня: свободно/занято/ваша
//       бронь (?date=YYYY-MM-DD, отсутствие — сегодня UTC); заблокированный
//       деканатом слот — «занято» (шаг 3); несуществующая аудитория → 404;
//       невалидная дата → 400 bad_request;
//   POST /v1/rooms/{id}/bookings — забронировать интервал (body: starts_at,
//       ends_at — 02 §5.2); двойная бронь того же слота → 400, пересечение
//       с активной бронью → 409 conflict, с блокировкой деканата →
//       409 blocked (шаг 3), невалидное тело → 400, комнаты нет → 404,
//       создано → 201; исключение БД (UNIQUE-гонка) → 400, не 500;
//   GET /v1/room-bookings        — мои активные брони (?upcoming=1 —
//       только предстоящие);
//   DELETE /v1/room-bookings/{id} — отмена своей активной брони → 204;
//       OwnershipGuard::check(Table::RoomBookings) ДО любых данных (правило
//       код-ревью OwnershipGuard.h; шаг 8 автотеста 02 §9.3 / 01 §5.10
//       «чужая бронь по подставленному адресу → 403»): Own → 204-цепочка,
//       NotFound → 404, Forbidden → 403 «Доступ только к своим данным»
//       (дословный пример 02 §5.1), Error → 500 fail-closed. Повторный
//       DELETE своей уже отменённой → 409 invalid_status_transition;
//   POST /v1/staff/rooms/{id}/block — блокировка интервала деканатом
//       (шаг 3; 02 §5.2 body: starts_at, ends_at, reason) — LoginFilter +
//       StaffFilter (02 §6.3 RBAC: студент → 403 «Доступ только для
//       сотрудников»; фильтры в METHOD_LIST после LoginFilter — роль из
//       сессии): Created → 201 (пересекавшиеся активные брони сняты,
//       авторам — уведомления, в одной транзакции), невалидное тело/пустой
//       reason → 400, комнаты нет → 404, ошибка БД → 500 (rollback).
//       OwnershipGuard НЕ вызывается: аудитория — общий ресурс, блокирует
//       её сотрудник, а не владеет (как staff-очередь Э5); личный объект —
//       бронь, её охраняет guard на DELETE выше.
//
// Ошибки — единый формат 02 §5.1. 409 контракта block (02 §5.2) в текущем
// сценарии не возникает: блокировка СНЯТАЕТ пересекшиеся брони, а не
// отклоняется (01 §5.10) — конфликтов-отказов нет; 400 — тело, 404 —
// комната (фиксируется честно).
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
    ADD_METHOD_TO(RoomsCtrl::blockRoom, "/v1/staff/rooms/{id}/block", Post,
                  "LoginFilter", "StaffFilter");
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
    void blockRoom(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb,
                   int64_t roomId);
};
