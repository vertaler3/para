#pragma once

#include <drogon/HttpController.h>

#include "services/OwnershipGuard.h"  // Table::Grades — guard для grades/{id}
#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Успеваемость (Э4; 02 §5.2). Тонкий контроллер (AGENTS.md §5.5): SQL и
// логика — GradesService; маршруты под LoginFilter — без cookie 401 от
// фильтра (02 §6.3). user_id — только из сессии.
//
//   GET /v1/grades          — свои оценки списком (path-id нет);
//   GET /v1/grades/summary  — сводка семестра (path-id нет);
//   GET /v1/grades/{id}     — одна запись: ПЕРВЫЙ path-id эндпоинт —
//       OwnershipGuard::check(Table::Grades) ДО любой отдачи данных
//       (правило код-ревью в шапке OwnershipGuard.h; 02 §6.3 п.3):
//       Own → 200, NotFound → 404, Forbidden → 403 (чужая запись),
//       Error → 500; выборка после guard'а — по PK без user_id-условия,
//       владение проверяется ровно один раз (постановка Э4 шаг 2);
//   GET /v1/exams           — «Сессия» из session_plan (path-id нет).
//
// /v1/grades/{id} и /v1/grades/summary не конфликтуют: статический сегмент
// router'а Drogon приоритетнее {id} (сверено E2E этого шага).
class GradesCtrl : public HttpController<GradesCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GradesCtrl::getGrades, "/v1/grades", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getSummary, "/v1/grades/summary", Get,
                  "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getGrade, "/v1/grades/{id}", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getExams, "/v1/exams", Get, "LoginFilter");
    METHOD_LIST_END

    void getGrades(const HttpRequestPtr& req,
                   std::function<void(const HttpResponsePtr&)>&& cb);
    void getSummary(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
    void getGrade(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t gradeId);
    void getExams(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
};
