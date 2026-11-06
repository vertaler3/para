#pragma once

#include <json/json.h>

#include <drogon/WebSocketConnection.h>

#include <cstddef>
#include <cstdint>

namespace para {

// Реестр живых WS-соединений «user_id → connection» (02 §3.2 поток 3:
// «WsCtrl проверяет сессию, регистрирует соединение в реестре user_id →
// connection. NotifyDispatcher пишет уведомление в БД — источник истины —
// и пушит живым соединениям»). В памяти процесса, под мьютексом —
// вызывается с IO-потоков drogon.
//
// NotifyDispatcher::setPushFn связан с send() в main.cpp (шаг 3 Э6);
// WsCtrl регистрирует соединение по user_id сессии (атрибут «session»
// от LoginFilter) и удаляет по закрытию. Несколько вкладок одного
// пользователя — несколько соединений: событие получают все живые.
class WsRegistry
{
  public:
    using ConnectionPtr = drogon::WebSocketConnectionPtr;

    // Зарегистрировать новое соединение пользователя (handleNewConnection).
    static void add(int64_t userId, const ConnectionPtr& conn);

    // Убрать соединение по закрытию (handleConnectionClosed); повторное
    // подключение создаёт новую запись — дедупликация по указателю:
    // чужое/несуществующее соединение — no-op.
    static void remove(int64_t userId, const ConnectionPtr& conn);

    // Пуш события JSON живым соединениям пользователя (payload —
    // {type,title,body,deep_link}, форма NotifyDispatcher::pushLive).
    // false — живых соединений нет (событие ничего не теряет: БД —
    // источник истины, лента отдаёт его при следующей выборке).
    // Мёртвые (disconnected) соединения не считаются доставленными и
    // лениво удаляются из реестра (страховка к обработчику закрытия).
    static bool send(int64_t userId, const Json::Value& payload);

    // Диагностика/тесты: всего соединений и соединений пользователя.
    static std::size_t size();
    static std::size_t countOf(int64_t userId);
};

}  // namespace para
