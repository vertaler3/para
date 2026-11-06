#include "controllers/WsCtrl.h"

#include <memory>

#include <drogon/drogon.h>

#include "services/SessionService.h"
#include "services/WsRegistry.h"

using namespace drogon;

void WsCtrl::handleNewConnection(const HttpRequestPtr& req,
                                 const WebSocketConnectionPtr& conn)
{
    // user_id кладёт LoginFilter в атрибут «session» (SessionInfo).
    // Fail-closed: атрибута нет → не регистрируем (паразитных соединений
    // без пользователя в реестре не бывает; Attributes::get при отсутствии
    // возвращает T() — проверено на StaffFilter, Э5 шаг 3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    if (session.userId <= 0)
    {
        LOG_ERROR << "WsCtrl: handshake без атрибута сессии — не регистрируем";
        conn->shutdown();  // 1000 normal closure
        return;
    }
    conn->setContext(std::make_shared<int64_t>(session.userId));
    para::WsRegistry::add(session.userId, conn);
}

void WsCtrl::handleConnectionClosed(const WebSocketConnectionPtr& conn)
{
    // UserId хранится в context соединения (setContext в handleNewConnection);
    // без context — не было регистрации, чистить нечего.
    if (!conn->hasContext())
        return;
    const auto userId = conn->getContext<int64_t>();
    para::WsRegistry::remove(*userId, conn);
}

void WsCtrl::handleNewMessage(const WebSocketConnectionPtr&,
                              std::string&&,
                              const WebSocketMessageType&)
{
    // Канал однонаправленный (push от сервера); кадры клиента игнорируем.
}
