#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Бронь аудитории (5.10; Э5а шаг 1; 02 §5.2). Тонкий контроллер
// (AGENTS.md §5.5): SQL и сетка слотов — RoomsService; роуты под
// LoginFilter — без cookie 401 от фильтра (02 §6.3). user_id — только
// из сессии.
//
//   GET /v1/rooms              — каталог аудиторий (path-id нет);
//   GET /v1/rooms/{id}/slots   — сетка слотов дня: свободно/занято/ваша
//       бронь (?date=YYYY-MM-DD, отсутствие — сегодня UTC); несуществующая
//       аудитория → 404 not_found; невалидная дата → 400 bad_request;
//       ошибка БД → 500 (fail-closed). Ошибки — единый формат 02 §5.1.
//
// OwnershipGuard ЗДЕСЬ НЕ ПРИМЕНЯЕТСЯ — сознательно: guard проверяет
// владение ЛИЧНЫМ объектом по path-id (заявка/оценка/уведомление/бронь —
// AGENTS.md §5.3); аудитория — общий ресурс каталога, её id чужим не
// бывает, а состояния слотов («занято» чужой бронью) читаются всеми
// по смыслу 5.10. Владение бронью — объект room_bookings, проверяется
// на его эндпоинтах (шаги 2–3: DELETE чужой брони → 403 — guard по
// таблице броней; правило код-ревью OwnershipGuard.h — про path-id
// личного объекта). Запись проверок шага 8 security-теста (403 на чужую
// бронь) — на эндпоинте брони, не сетки.
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
    METHOD_LIST_END

    void getRooms(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void getSlots(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t roomId);
};
