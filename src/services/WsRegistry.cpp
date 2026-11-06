#include "services/WsRegistry.h"

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace para
{
namespace
{

std::mutex gMutex;
// user_id → соединения (несколько вкладок — несколько записей).
std::unordered_map<int64_t, std::vector<WsRegistry::ConnectionPtr>> gConns;

// Компактная сериализация события (одна строка — один кадр WS); emitUTF8 —
// кириллица сырыми байтами (правки по гейту Э6 раунд 1: сырой поиск
// «История России» в кадре; json-парсеры декодируют обе формы одинаково).
std::string toJsonString(const Json::Value& payload)
{
    Json::StreamWriterBuilder b;
    b["indentation"] = "";  // без переводов строк — payload как есть
    b["emitUTF8"] = true;
    return Json::writeString(b, payload);
}

}  // namespace

void WsRegistry::add(int64_t userId, const ConnectionPtr& conn)
{
    if (userId <= 0 || !conn)
        return;
    std::lock_guard<std::mutex> lock(gMutex);
    gConns[userId].push_back(conn);
}

void WsRegistry::remove(int64_t userId, const ConnectionPtr& conn)
{
    if (userId <= 0 || !conn)
        return;
    std::lock_guard<std::mutex> lock(gMutex);
    const auto it = gConns.find(userId);
    if (it == gConns.end())
        return;  // соединение не наше/не регистрировалось — no-op
    auto& list = it->second;
    list.erase(std::remove(list.begin(), list.end(), conn), list.end());
    if (list.empty())
        gConns.erase(it);  // пользователь офлайн — запись не держим
}

bool WsRegistry::send(int64_t userId, const Json::Value& payload)
{
    if (userId <= 0 || payload.isNull())
        return false;
    const std::string frame = toJsonString(payload);
    std::lock_guard<std::mutex> lock(gMutex);
    const auto it = gConns.find(userId);
    if (it == gConns.end())
        return false;
    bool delivered = false;
    auto& list = it->second;
    auto out = list.begin();
    for (auto iter = list.begin(); iter != list.end(); ++iter)
    {
        const ConnectionPtr& conn = *iter;
        if (conn && conn->connected())
        {
            conn->send(frame);  // текстовый кадр RFC 6455 (дефолт Text)
            delivered = true;
            *out++ = conn;
        }
        // Мёртвое соединение выпадает из реестра (ленивая чистка к
        // обработчику закрытия — страхование от пропущенного колбэка).
    }
    list.erase(out, list.end());
    if (list.empty())
        gConns.erase(it);
    return delivered;
}

std::size_t WsRegistry::size()
{
    std::lock_guard<std::mutex> lock(gMutex);
    std::size_t total = 0;
    for (const auto& [userId, list] : gConns)
        total += list.size();
    return total;
}

std::size_t WsRegistry::countOf(int64_t userId)
{
    std::lock_guard<std::mutex> lock(gMutex);
    const auto it = gConns.find(userId);
    return it == gConns.end() ? 0 : it->second.size();
}

}  // namespace para
