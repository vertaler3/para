#pragma once

#include <drogon/HttpController.h>

#include "services/OwnershipGuard.h"  // Table::Applications — guard для {id}
#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// Заявки студента (Э5 шаг 2; 02 §5.2). Тонкий контроллер (AGENTS.md §5.5):
// парсинг тела, guard, маппинг исходов сервиса в HTTP — максимум; вся SQL
// и логика — ApplicationsService (+ автомат AppWorkflow шага 1). Маршруты
// под LoginFilter — без cookie 401 от фильтра (02 §6.3); user_id — только
// из сессии.
//
//   GET  /v1/application-types          — каталог типов (path-id нет);
//   POST /v1/applications               — подача: черновик | сразу submitted
//                                         + событие таймлайна (201 · 400);
//   GET  /v1/applications               — свои заявки, query status (200);
//   GET  /v1/applications/{id}          — своя заявка + таймлайн;
//   PUT  /v1/applications/{id}          — автосохранение черновика (С7),
//                                         только draft → иначе 409;
//   POST /v1/applications/{id}/messages — ответ в мини-диалоге (С7), текст
//                                         в таймлайн; need_info → in_progress
//                                         через автомат (С7.6).
//
// КАЖДЫЙ path-id — через OwnershipGuard::check(Table::Applications) ДО любой
// отдачи данных и ДО проверки статуса (правило код-ревью в шапке
// OwnershipGuard.h; постановка шага 2: PUT/messages — guard до статуса):
// Own → обработка, NotFound → 404, Forbidden → 403 (чужая заявка),
// Error → 500 fail-closed. Формат ошибок — 02 §5.1 {error:{code,message}}.
// GET /v1/applications/{id}/pdf (PdfBuilder) — отдельный шаг Э5: тот же
// guard-до-статуса порядок.
class ApplicationsCtrl : public HttpController<ApplicationsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ApplicationsCtrl::getTypes, "/v1/application-types", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::create, "/v1/applications", Post,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::list, "/v1/applications", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::getOne, "/v1/applications/{id}", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::updateDraft, "/v1/applications/{id}", Put,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::sendMessage,
                  "/v1/applications/{id}/messages", Post, "LoginFilter");
    METHOD_LIST_END

    void getTypes(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void create(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb);
    void list(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void getOne(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb,
                int64_t applicationId);
    void updateDraft(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb,
                     int64_t applicationId);
    void sendMessage(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb,
                     int64_t applicationId);
};
