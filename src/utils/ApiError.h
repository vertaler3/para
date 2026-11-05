#pragma once

#include <drogon/HttpResponse.h>

#include <string>

namespace para::api
{

// Единый формат ошибки API — 02 §5.1: {"error":{"code":"...","message":"..."}}.
// message — короткая человекочитаемая фраза БЕЗ деталей: для 401 не
// раскрывается, что именно неверно (логин или пароль), для 500 — без
// служебных строк и стека (детали только в серверном логе).
// HTTP-коды — те же, что в плоском формате (02 §5.1, список кодов).
// Только для JSON-плоскости /v1/; htmx-фрагменты об ошибках — с Э2
// (02 §5.1: content negotiation по HX-Request).
inline drogon::HttpResponsePtr errorResponse(const std::string& code,
                                             const std::string& message,
                                             drogon::HttpStatusCode status)
{
    Json::Value err;
    err["code"] = code;
    err["message"] = message;
    Json::Value body;
    body["error"] = std::move(err);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(body));
    resp->setStatusCode(status);
    return resp;
}

}  // namespace para::api
