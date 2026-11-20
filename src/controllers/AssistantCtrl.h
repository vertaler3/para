#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// AI-ассистент (идея 5.19, Р-36). Тонкий контроллер (AGENTS.md §5.5):
// разбор тела, порядок проверок и маппинг исходов AssistantService в HTTP;
// контекст, история, вызов модели и SQL — в сервисе. Оба роута под
// LoginFilter (без cookie → 401, 02 §6.3); POST проходит CSRF-проверку
// LoginFilter автоматически (Р-23). Path-id нет — всё по user_id сессии.
//
//   POST /v1/assistant/chat {message} — порядок проверок фиксирован:
//       1) тело/поле message: пусто → 400 bad_request;
//       2) > 700 кодпоинтов → 400 bad_request (до лимитера — длинный
//          вход не тратит слот и не маскируется 429);
//       3) лимит 10/мин на пользователя → 429 rate_limited + Retry-After
//          (слот учитывается ДО вызова модели: 429 срабатывает и тогда,
//          когда модель отвечает ошибкой);
//       4) сервис: 200 {reply} · 500 internal (БД) ·
//          502 assistant_unavailable (модель недоступна / не настроена).
//   GET  /v1/assistant/history — последние 20 своих сообщений
//       {messages:[{id, role, content, created_at}]} по возрастанию.
class AssistantCtrl : public HttpController<AssistantCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AssistantCtrl::chat, "/v1/assistant/chat", Post,
                  "LoginFilter");
    ADD_METHOD_TO(AssistantCtrl::history, "/v1/assistant/history", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void chat(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
    void history(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& cb);
};
