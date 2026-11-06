#pragma once

#include <drogon/HttpResponse.h>

#include <string>
#include <utility>

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

// Успешный JSON-ответ с человекочитаемым UTF-8 (Э6 правки по гейту, раунд 1).
// newHttpJsonResponse сериализует jsoncpp-ом с дефолтами: не-ASCII
// экранируется \uXXXX («История России» → "\u0418...") — тело корректно для
// json-парсера, но сырой поиск кириллицы в ответе не работает (поймано
// гейтом Э6: перенос/дашборд/лента false при сыром поиске дисциплины и
// заголовка). Здесь StreamWriterBuilder["emitUTF8"]=true — кириллица идёт
// сырыми UTF-8 байтами; JSON-семантика не меняется (парсеры декодируют
// обе формы одинаково).
inline drogon::HttpResponsePtr jsonResponse(Json::Value body,
                                            drogon::HttpStatusCode status =
                                                drogon::k200OK)
{
    Json::StreamWriterBuilder b;
    b["emitUTF8"] = true;
    b["indentation"] = "";  // компактная форма — как у newHttpJsonResponse
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(status);
    resp->setContentTypeString("application/json");
    resp->setBody(Json::writeString(b, body));
    return resp;
}

}  // namespace para::api
