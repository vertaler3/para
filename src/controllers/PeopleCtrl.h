#pragma once

#include <drogon/HttpController.h>

#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Каталог людей (Э6а, 5.11; 02 §5.2: /v1/people*). Тонкий контроллер
// (AGENTS.md §5.5): SQL и правило видимости — PeopleService; роуты под
// LoginFilter — без cookie 401 от фильтра (02 §6.3).
//
//   GET /v1/people/search        — поиск: query ?q=, ?role= (teacher|
//       student|staff — иное → 400), ?faculty= (применяется только к
//       студентам); преподаватели/сотрудники — все, студенты — только
//       своего факультета (факультет запрашивающего-студента — из БД по
//       сессии, не из query; staff — все);
//   GET /v1/people/{id}          — профиль: преподаватель/сотрудник —
//       полный; студент — свой полный, чужой — ТОЛЬКО публичные поля
//       (имя, группа, почта) — 02 §5.2 «Правила видимости 5.11–5.16».
//       403 в этом роуте НЕ возникает: смотреть профиль и расписание
//       группы чужого студента легально (01 §5.11 — граница приватности),
//       оценки/заявки/брони чужих people-эндпоинты не отдают вовсе, а
//       DoD №2 (403 на чужой id) остаётся в силе на их собственных
//       роутах (grades/applications/room-bookings — OwnershipGuard);
//   GET /v1/people/{id}/schedule — занятия преподавателя / расписание
//       группы студента (публично).
//
// Адресация {id} — people-пространство (Р-15 в docs/decisions.md): id <
// 1000 — студент (users.id), id ≥ 1000 — карточка teacher_profiles
// (id − 1000); staff-юзер под своим users.id → 404 (карточка — в
// teacher_profiles, Р-14); несуществующий → 404; ошибка БД → 500
// fail-closed. Нечисловой id не сматчится параметром int64_t — drogon
// отвечает 400 (как /v1/grades/{id}, сверено E2E в Э4).
//
// Роуты не конфликтуют: /v1/people/search — статический сегмент 3-го
// уровня, {id} и {id}/schedule — 3-й и 4-й; статический приоритетнее
// (сверено E2E в Э4 для /grades/summary).
class PeopleCtrl : public HttpController<PeopleCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PeopleCtrl::search, "/v1/people/search", Get,
                  "LoginFilter");
    ADD_METHOD_TO(PeopleCtrl::person, "/v1/people/{id}", Get, "LoginFilter");
    ADD_METHOD_TO(PeopleCtrl::schedule, "/v1/people/{id}/schedule", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void search(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb);
    void person(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb,
                int64_t personId);
    void schedule(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t personId);
};
