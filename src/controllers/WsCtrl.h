#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

// Приватный WebSocket-канал push (Э6 шаг 3; 02 §3.2 поток 3, §5.2:
// GET /ws → 101 upgrade · 401). Тонкий контроллер: сессию проверяет
// LoginFilter (cookie para_session → SessionService::findValid; без
// валидной сессии handshake отклоняется 401 в формате 02 §5.1 ДО апгрейда),
// реестр — WsRegistry, пуш — NotifyDispatcher (связка в main.cpp).
class WsCtrl : public drogon::WebSocketController<WsCtrl>
{
  public:
    WS_PATH_LIST_BEGIN
    // «/ws» НЕ в публичном списке LoginFilter — handshake требует сессии.
    WS_PATH_ADD("/ws", "LoginFilter", Get);
    WS_PATH_LIST_END

    // Установлено: user_id из атрибута «session» (LoginFilter) →
    // WsRegistry::add; идентификатор пользователя — в context соединения
    // (для чистки при закрытии).
    void handleNewConnection(const HttpRequestPtr& req,
                             const WebSocketConnectionPtr& conn) override;

    // Закрыто: соединение удаляется из реестра; повторное подключение —
    // новая запись (шаг 3 Э6).
    void handleConnectionClosed(const WebSocketConnectionPtr& conn) override;

    // Входящие кадры клиента протоколом канала не предусмотрены —
    // игнорируются (ping/pong фреймворк обрабатывает сам).
    void handleNewMessage(const WebSocketConnectionPtr&,
                          std::string&&,
                          const WebSocketMessageType&) override;
};
