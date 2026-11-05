#include "HealthCtrl.h"

void HealthCtrl::asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    // Мониторинг (02 §8.4) и early check первого вечера Э1: тело строго «ok».
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setBody("ok");
    callback(resp);
}
