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
//
// Страничная плоскость (Э7, экран «Заявки» 1 из 4; путь вне /v1/ — по
// постановке 02 §3.2, как /schedule и /grades — Р-19):
//   GET /applications         — views/applications.csp: список «Мои заявки»,
//                               фильтр ?status= (v3, fe_rebuild шаг 12):
//                               группа active|reply|ready|issued|draft|closed
//                               ИЛИ прежнее значение kApplicationStatuses
//                               (сводится к своей группе). Сервис отдаёт
//                               ПОЛНЫЙ список, группа только предвыбирает
//                               фильтр (клиентский фильтр и поиск — app.js).
//                               НЕИЗВЕСТНОЕ → 303 на /applications (страница
//                               не отвечает голым JSON 400);
//   GET /applications/new     — views/application_form.csp: каталог типов +
//                               форма 1–2 вопроса (данные грузит клиент:
//                               GET /v1/application-types и /v1/me).
//                               path-id НЕТ — OwnershipGuard не применим;
//                               статический сегмент new приоритетнее {id}
//                               (прецедент /v1/grades/summary — сверено E2E);
//   GET /applications/{id}    — views/application.csp: карточка заявки с
//                               таймлайном. РЕШЕНИЕ ПО GUARD (path-id, задоку-
//                               ментировано по постановке экрана): вызывается
//                               OwnershipGuard::check(Table::Applications)
//                               ДО любой отдачи данных (правило код-ревью в
//                               шапке OwnershipGuard.h; владение проверяется
//                               ровно один раз — выборка after-Own идёт по PK
//                               без user_id-условия, как в getOne). Вердикты
//                               для СТРАНИЦЫ (03 §5.3): Forbidden → 403 и
//                               NotFound → 404 — РАЗНЫЕ коды статуса (DoD 01
//                               п.2: 403 на чужой id — проверяемо жюри), но
//                               ОДНА нейтральная карточка «Не нашли то, что
//                               искали» — текст пользователя не различает
//                               чужое/несуществующее (03 §5.3:1021); это HTML-
//                               страница с кодом ошибки, НЕ голый JSON
//                               (постановка Э7); Error → 500 «Сервис не
//                               отвечает» (fail-closed). Тела страницы — те
//                               же методы ApplicationsService, что за JSON
//                               API (SQL в контроллере нет, AGENTS §5.5).
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
    ADD_METHOD_TO(ApplicationsCtrl::listPage, "/applications", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::newPage, "/applications/new", Get,
                  "LoginFilter");
    ADD_METHOD_TO(ApplicationsCtrl::cardPage, "/applications/{id}", Get,
                  "LoginFilter");
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
    void listPage(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb);
    void newPage(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
    void cardPage(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& cb,
                  int64_t applicationId);
};
