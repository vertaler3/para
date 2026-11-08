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
//
// Страничная плоскость (Э7, экран «Успеваемость»; 02 §5.2: SSR-страница
// /grades; путь вне /v1/ — по постановке 02 §3.2, как /schedule — Р-19):
//   GET /grades                          — views/grades.csp (03 §4.4);
//   GET /grades/discipline/{discipline_id} — views/grades-discipline.csp
//       (03 §4.9.3). ВЛАДЕНИЕ на этом path-id — решение шага:
//       discipline_id — НЕ строка таблицы grades, а группировка СВОИХ
//       оценок; OwnershipGuard::check(Table::Grades) сторожит id СТРОКИ
//       оценки (GET /v1/grades/{id}) и к группировке неприменим. Выборка
//       идёт через GradesService::list по user_id СЕССИИ (02 §6.3) —
//       среди своих оценок дисциплины с чужим/несуществующим id нет, обе
//       ситуации → 404 not_found без различия (03 §5.3: существование
//       чужих объектов не раскрываем). SQL в контроллере нет (AGENTS
//       §5.5): тела страниц — те же методы сервиса, что за JSON API.
class GradesCtrl : public HttpController<GradesCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GradesCtrl::getGrades, "/v1/grades", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getSummary, "/v1/grades/summary", Get,
                  "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getGrade, "/v1/grades/{id}", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::getExams, "/v1/exams", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::gradesPage, "/grades", Get, "LoginFilter");
    ADD_METHOD_TO(GradesCtrl::disciplinePage,
                  "/grades/discipline/{discipline_id}", Get, "LoginFilter");
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
    void gradesPage(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& cb);
    void disciplinePage(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& cb,
                        int64_t disciplineId);
};
