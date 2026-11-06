#pragma once

#include <drogon/HttpController.h>

#include "services/OwnershipGuard.h"  // Table::Applications — guard {id}
#include "services/SessionService.h"  // SessionInfo — атрибут LoginFilter

using namespace drogon;

// PDF-справка заявки (Э5 шаг 4; 02 §5.2: GET /v1/applications/{id}/pdf —
// 200 · 403 · 404 · 409 (ещё не готова); поток 5 §3.2; 02 §4.4/дерево 05 §2
// — отдельный PdfCtrl поверх libharu). Тонкий: guard → статус → сборка —
// вся генерация в PdfBuilder.
//
// Порядок проверки — постановка шага 4: OwnershipGuard::check
// (Table::Applications) ДО проверки статуса: чужая заявка → 403, даже если
// она и не ready|issued; несуществующая → 404. Собственная заявка не в
// ready|issued → 409 not_ready («ещё не готова», 02 §5.2). ready|issued →
// 200 application/pdf (TTF кириллица встроен, диагональный штамп «ДЕМО»).
class PdfCtrl : public HttpController<PdfCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PdfCtrl::getPdf, "/v1/applications/{id}/pdf", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void getPdf(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& cb,
                int64_t applicationId);
};
