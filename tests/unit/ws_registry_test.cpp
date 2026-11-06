// Юнит-тесты WsRegistry (Э6 шаг 3; 02 §3.2 поток 3: реестр user_id →
// connection, пуш живым соединениям, чистка при закрытии):
//  - add/send: событие {type,title,body,deep_link} доставляется живому
//    соединению одним текстовым кадром (валидный JSON, компактный);
//  - несколько вкладок одного пользователя — получают все; remove одной
//    не задевает остальные;
//  - remove: чужое/несуществующее соединение — no-op; последний — запись
//    удаляется;
//  - мёртвое соединение: send не считает доставленным и лениво чистит;
//  - send несуществующему пользователю → false.
// WebSocketConnection — абстрактный класс с виртуальными методами (v1.9.14,
// WebSocketConnection.h) — фейк-наследник без сети; двойной move-паттерн
// и статические состояния реестра сбрасываются меж кейсами (реристр —
// синглтон в процессе: после каждого TEST_CASE не должно остаться записей).
#include <json/json.h>

#include <drogon/WebSocketConnection.h>

#include <chrono>
#include <string>
#include <vector>

#include <doctest.hpp>

#include "services/WsRegistry.h"

using drogon::WebSocketConnection;
using drogon::WebSocketConnectionPtr;
using drogon::WebSocketMessageType;
using para::WsRegistry;

namespace {

// Фейк, сохраняющий кадры в вектор владельца (владелец живёт дольше shared_ptr).
struct CapturingConnection : public WebSocketConnection
{
    std::vector<std::string>* frames;
    bool open = true;
    explicit CapturingConnection(std::vector<std::string>* f) : frames(f) {}

    void send(const char* msg, uint64_t len,
              const WebSocketMessageType) override
    {
        frames->emplace_back(msg, len);
    }
    void send(std::string_view msg, const WebSocketMessageType) override
    {
        frames->emplace_back(msg);
    }
    void sendJson(const Json::Value&,
                  const WebSocketMessageType) override
    {
    }
    const trantor::InetAddress& localAddr() const override
    {
        static const trantor::InetAddress addr("127.0.0.1", 0);
        return addr;
    }
    const trantor::InetAddress& peerAddr() const override
    {
        static const trantor::InetAddress addr("127.0.0.1", 0);
        return addr;
    }
    bool connected() const override { return open; }
    bool disconnected() const override { return !open; }
    void shutdown(const drogon::CloseCode, const std::string&) override
    {
        open = false;
    }
    void forceClose() override { open = false; }
    void setPingMessage(const std::string&,
                        const std::chrono::duration<double>&) override
    {
    }
    void disablePing() override {}
};

Json::Value eventPayload()
{
    Json::Value p(Json::objectValue);
    p["type"] = "schedule";
    p["title"] = "Пара в 10:00 перенесена в ауд. 4210";
    p["body"] = "Мировая экономика, 4 ноября: ауд. 3024 → 4210";
    p["deep_link"] = "/schedule?date=2026-11-04";
    return p;
}

}  // namespace

TEST_CASE("WsRegistry: push живому соединению — валидный JSON-кадр")
{
    std::vector<std::string> frames;
    auto conn = std::make_shared<CapturingConnection>(&frames);
    WsRegistry::add(1, conn);
    CHECK(WsRegistry::countOf(1) == 1);
    CHECK(WsRegistry::size() == 1);

    CHECK(WsRegistry::send(1, eventPayload()));

    REQUIRE(frames.size() == 1);
    // Кадр — компактный JSON ровно с полями события (форма pushLive, 02 §2.4).
    Json::Value parsed;
    REQUIRE(Json::Reader().parse(frames[0], parsed));
    CHECK(parsed["type"].asString() == "schedule");
    CHECK(parsed["title"].asString() ==
          "Пара в 10:00 перенесена в ауд. 4210");
    CHECK(parsed["body"].asString() ==
          "Мировая экономика, 4 ноября: ауд. 3024 → 4210");
    CHECK(parsed["deep_link"].asString() == "/schedule?date=2026-11-04");
    CHECK(frames[0].find('\n') == std::string::npos);  // один кадр — одна строка

    // Чистка между кейсами (реестр — синглтон процесса).
    WsRegistry::remove(1, conn);
    CHECK(WsRegistry::size() == 0);
}

TEST_CASE("WsRegistry: несколько вкладок — получают все, remove адресный")
{
    std::vector<std::string> frames1, frames2;
    auto conn1 = std::make_shared<CapturingConnection>(&frames1);
    auto conn2 = std::make_shared<CapturingConnection>(&frames2);
    WsRegistry::add(1, conn1);
    WsRegistry::add(1, conn2);
    CHECK(WsRegistry::countOf(1) == 2);

    CHECK(WsRegistry::send(1, eventPayload()));
    CHECK(frames1.size() == 1);
    CHECK(frames2.size() == 1);

    // Закрылась первая вкладка — вторая продолжает получать.
    WsRegistry::remove(1, conn1);
    CHECK(WsRegistry::countOf(1) == 1);
    CHECK(WsRegistry::send(1, eventPayload()));
    CHECK(frames1.size() == 1);  // закрытой не доставляется
    CHECK(frames2.size() == 2);

    // Последняя закрыта — запись пользователя исчезает.
    WsRegistry::remove(1, conn2);
    CHECK(WsRegistry::size() == 0);
    CHECK_FALSE(WsRegistry::send(1, eventPayload()));
}

TEST_CASE("WsRegistry: remove чужого/несуществующего — no-op")
{
    std::vector<std::string> frames;
    auto conn = std::make_shared<CapturingConnection>(&frames);
    WsRegistry::add(1, conn);

    WsRegistry::remove(2, conn);  // соединение другого пользователя
    CHECK(WsRegistry::countOf(1) == 1);

    WsRegistry::remove(1, nullptr);  // нулевой указатель — no-op
    WsRegistry::remove(999, conn);   // незарегистрированный пользователь
    CHECK(WsRegistry::countOf(1) == 1);

    // После корректного remove повторный remove того же соединения — no-op.
    WsRegistry::remove(1, conn);
    CHECK(WsRegistry::size() == 0);
    WsRegistry::remove(1, conn);
    CHECK(WsRegistry::size() == 0);
}

TEST_CASE("WsRegistry: мёртвое соединение — не доставляется и чистится")
{
    std::vector<std::string> frames;
    auto alive = std::make_shared<CapturingConnection>(&frames);
    auto dead = std::make_shared<CapturingConnection>(&frames);
    dead->open = false;
    WsRegistry::add(1, alive);
    WsRegistry::add(1, dead);
    CHECK(WsRegistry::countOf(1) == 2);

    CHECK(WsRegistry::send(1, eventPayload()));  // живому доставлено
    CHECK(frames.size() == 1);                   // мёртвому кадра нет
    // Ленивая чистка: от мёртвого реестр избавился сам.
    CHECK(WsRegistry::countOf(1) == 1);
    CHECK(WsRegistry::size() == 1);

    // Все мёртвые — send возвращает false (доставки не было).
    alive->open = false;
    CHECK_FALSE(WsRegistry::send(1, eventPayload()));
    CHECK(frames.size() == 1);
    CHECK(WsRegistry::size() == 0);
}

TEST_CASE("WsRegistry: send пользователю без соединений и граничные случаи")
{
    CHECK_FALSE(WsRegistry::send(42, eventPayload()));  // не в реестре
    WsRegistry::add(0, nullptr);  // невалидные входы — не регистрируются
    WsRegistry::add(-1, nullptr);
    CHECK(WsRegistry::size() == 0);
    CHECK_FALSE(WsRegistry::send(7, Json::Value()));  // null payload
}
