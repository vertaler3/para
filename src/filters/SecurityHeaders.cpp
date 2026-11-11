#include "filters/SecurityHeaders.h"

#include <drogon/HttpAppFramework.h>

namespace para
{

void SecurityHeaders::apply(const drogon::HttpResponsePtr& resp)
{
    if (!resp)
        return;
    // Set-if-absent: addHeader добавляет, не заменяет (урок Э3 — двойной
    // Content-Type у /ics). На свежем ответе из фабрики заголовков ещё нет,
    // для кэшируемых ответов (404-заглушка) гвард делает вызов идемпотентным.
    if (resp->getHeader(kContentTypeOptions).empty())
        resp->addHeader(kContentTypeOptions, kContentTypeOptionsValue);
    if (resp->getHeader(kFrameOptions).empty())
        resp->addHeader(kFrameOptions, kFrameOptionsValue);
    if (resp->getHeader(kCsp).empty())
        resp->addHeader(kCsp, kCspValue);
    if (resp->getHeader(kReferrerPolicy).empty())
        resp->addHeader(kReferrerPolicy, kReferrerPolicyValue);
}

void SecurityHeaders::registerAdvice()
{
    // Advice на СОЗДАНИЕ ответа — самая ранняя и самая широкая точка:
    // срабатывает во всех фабриках HttpResponse (см. список в шапке
    // SecurityHeaders.h) до любых последующих операций над ответом,
    // поэтому заголовки не могут быть «потеряны» по пути к клиенту.
    drogon::app().registerHttpResponseCreationAdvice(
        [](const drogon::HttpResponsePtr& resp) { apply(resp); });
}

}  // namespace para
