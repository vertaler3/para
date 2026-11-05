#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace para {

// Лимитер попыток входа (02 §6.4; конкретизация 03 §4.1): после 3 неудач
// подряд — блок 15 с, каждая следующая неудача удваивает (15→30→60→120…),
// потолок 15 минут; успешный вход сбрасывает счётчик.
// Хранит состояние в памяти процесса; при изменениях AuthCtrl зеркально
// обновляет users.failed_attempts/locked_until (02 §4.1/§6.4 — состояние
// видно в БД; полное сохранение блокировки между рестартами — Э8).
// Часы передаются параметром — юнит-тесты без ожиданий.
class LoginLimiter
{
  public:
    static constexpr int kMaxFailures = 3;      // до первой блокировки
    static constexpr int64_t kInitialLockSec = 15;
    static constexpr int64_t kMaxLockSec = 15 * 60;  // потолок 15 минут

    // >0 — логин заблокирован, вернуто число оставшихся секунд; 0 — можно.
    int64_t blockedFor(const std::string& login, int64_t nowSec) const;

    // Зарегистрировать неудачу: после 3-й подряд включает/продлевает блок.
    void onFailure(const std::string& login, int64_t nowSec);

    // Успешный вход: сброс счётчика и блокировки.
    void onSuccess(const std::string& login);

    // Текущее число подряд идущих неудач (для зеркалирования в users).
    int failures(const std::string& login) const;

  private:
    struct Entry
    {
        int failures = 0;
        int64_t lockSec = 0;      // длительность последнего блока (для удвоения)
        int64_t lockedUntil = 0;  // unix-сек, 0 — не заблокирован
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Entry> entries_;
};

}  // namespace para
