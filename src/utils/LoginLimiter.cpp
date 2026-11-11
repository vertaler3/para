#include "utils/LoginLimiter.h"

namespace para {

int64_t LoginLimiter::blockedFor(const std::string& login,
                                 int64_t nowSec) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = entries_.find(login);
    if (it == entries_.end())
        return 0;
    const Entry& entry = it->second;
    if (entry.lockedUntil <= nowSec)
        return 0;
    return entry.lockedUntil - nowSec;
}

void LoginLimiter::onFailure(const std::string& login, int64_t nowSec)
{
    std::lock_guard<std::mutex> lock(mutex_);
    Entry& entry = entries_[login];
    ++entry.failures;
    if (entry.failures >= kMaxFailures)
    {
        // Первая блокировка — 15 с; далее удвоение от предыдущей
        // длительности с потолком 15 минут (03 §4.1: 15→30→60→120…).
        const int64_t next = entry.lockSec == 0
                                 ? kInitialLockSec
                                 : entry.lockSec * 2;
        entry.lockSec = next > kMaxLockSec ? kMaxLockSec : next;
        entry.lockedUntil = nowSec + entry.lockSec;
    }
}

void LoginLimiter::onSuccess(const std::string& login)
{
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.erase(login);
}

int LoginLimiter::failures(const std::string& login) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = entries_.find(login);
    return it == entries_.end() ? 0 : it->second.failures;
}

int64_t LoginLimiter::lockUntilSec(const std::string& login,
                                   int64_t nowSec) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = entries_.find(login);
    if (it == entries_.end())
        return 0;
    const Entry& entry = it->second;
    return entry.lockedUntil > nowSec ? entry.lockedUntil : 0;
}

}  // namespace para
