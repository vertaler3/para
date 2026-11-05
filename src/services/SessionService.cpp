#include "services/SessionService.h"

#include <array>
#include <cstdlib>
#include <ctime>

#include <openssl/rand.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {

std::string SessionService::sessionCookie(const std::string& token,
                                          int64_t ttlSec)
{
    // Единый хелпер выдачи cookie (02 §6.1): 128-битный токен; флаги
    // HttpOnly (недоступен JS), Secure (только по HTTPS за Nginx, 02 §6.5),
    // SameSite=Lax (первый барьер от CSRF — 02 §6.4; double-submit — Э8).
    return std::string(kCookieName) + "=" + token + "; Path=/; Max-Age=" +
           std::to_string(ttlSec) + "; HttpOnly; Secure; SameSite=Lax";
}

std::string SessionService::generateToken()
{
    // 128 бит CSPRNG → 32 hex-символа: в cookie — только он (02 §6.1).
    std::array<unsigned char, 16> raw{};
    if (RAND_bytes(raw.data(), static_cast<int>(raw.size())) != 1)
    {
        throw std::runtime_error("SessionService: RAND_bytes failed");
    }
    static const char* hex = "0123456789abcdef";
    std::string token;
    token.reserve(raw.size() * 2);
    for (const unsigned char byte : raw)
    {
        token.push_back(hex[byte >> 4]);
        token.push_back(hex[byte & 0x0F]);
    }
    return token;
}

std::string SessionService::utcNow()
{
    const std::time_t now = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&now, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

int64_t SessionService::currentTtlSec()
{
    const char* hours = std::getenv("PARA_SESSION_TTL_HOURS");
    if (hours != nullptr && *hours != '\0')
    {
        const long parsed = std::strtol(hours, nullptr, 10);
        if (parsed > 0)
            return static_cast<int64_t>(parsed) * 3600;
    }
    return kDefaultTtlSec;
}

void SessionService::create(
    drogon::orm::DbClientPtr db,
    int64_t userId,
    int64_t ttlSec,
    std::function<void(bool ok, const std::string& token)> cb)
{
    const std::string token = generateToken();
    const std::string now = utcNow();
    const std::time_t expires = std::time(nullptr) + ttlSec;
    std::tm tm{};
    gmtime_r(&expires, &tm);
    char expiresBuf[32];
    std::strftime(expiresBuf, sizeof(expiresBuf), "%Y-%m-%dT%H:%M:%SZ", &tm);

    // Чистка истёкших сессий заодно с созданием (idx_sessions_expiry).
    *db << "DELETE FROM sessions WHERE expires_at <= $1" << now >>
        [db, userId, token = std::move(token), now, expiresBuf,
         cb = std::move(cb)](const Result&) mutable {
            *db << "INSERT INTO sessions(id, user_id, created_at, expires_at) "
                   "VALUES($1, $2, $3, $4)"
                << token << userId << now << std::string(expiresBuf) >>
                [cb = std::move(cb), token](const Result&) {
                    cb(true, token);
                } >>
                [cb = std::move(cb)](const DrogonDbException& e) {
                    LOG_ERROR << "SessionService::create: " << e.base().what();
                    cb(false, "");
                };
        } >>
        [cb = std::move(cb)](const DrogonDbException& e) {
            LOG_ERROR << "SessionService::create (cleanup): "
                      << e.base().what();
            cb(false, "");
        };
}

void SessionService::findValid(
    drogon::orm::DbClientPtr db,
    const std::string& token,
    std::function<void(std::optional<SessionInfo>)> cb)
{
    *db << "SELECT s.user_id, u.login, u.role FROM sessions s "
           "JOIN users u ON u.id = s.user_id "
           "WHERE s.id = $1 AND s.expires_at > $2"
        << token << utcNow() >>
        [cb = std::move(cb)](const Result& rows) {
            if (rows.empty())
            {
                cb(std::nullopt);
                return;
            }
            SessionInfo info;
            info.userId = rows[0]["user_id"].as<int64_t>();
            info.login = rows[0]["login"].as<std::string>();
            info.role = rows[0]["role"].as<std::string>();
            cb(std::move(info));
        } >>
        [cb = std::move(cb)](const DrogonDbException& e) {
            LOG_ERROR << "SessionService::findValid: " << e.base().what();
            cb(std::nullopt);
        };
}

void SessionService::remove(drogon::orm::DbClientPtr db,
                            const std::string& token,
                            std::function<void()> cb)
{
    *db << "DELETE FROM sessions WHERE id = $1" << token >>
        [cb = std::move(cb)](const Result&) { cb(); } >>
        [cb = std::move(cb)](const DrogonDbException& e) {
            LOG_ERROR << "SessionService::remove: " << e.base().what();
            cb();
        };
}

}  // namespace para
