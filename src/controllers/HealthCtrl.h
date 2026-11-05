#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

// Путь /healthz вынесен в константу: её использует и роут ниже, и смок-тест
// tests/unit/healthz_smoke.cpp (шаг 2 Э1). Константа — не пользовательская
// строка, серверная.
inline constexpr const char* kHealthzPath = "/healthz";

// Тонкий контроллер мониторинга (10–30 строк, AGENTS.md §5.5):
// GET /healthz → HTTP 200, тело «ok».
class HealthCtrl : public drogon::HttpSimpleController<HealthCtrl>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    PATH_ADD(kHealthzPath, Get);
    PATH_LIST_END
};
