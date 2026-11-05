// Юнит-тесты аутентификации (шаг 7 Э1; 02 §9.1: PasswordHasher round-trip,
// лимитер входа — нарастающая задержка и сброс после успеха).
// Демо-пароль здесь НЕ используется: PasswordHasher тестируется на
// собственном тестовом пароле (демо-пароль в репо не попадает, AGENTS §5.2).
#include <doctest.hpp>

#include <string>

#include "services/PasswordHasher.h"
#include "utils/LoginLimiter.h"

using para::LoginLimiter;
using para::PasswordHasher;

TEST_CASE("PasswordHasher: round-trip argon2id, параметры OWASP в encoded")
{
    const std::string hash = PasswordHasher::hash("unit-test-password-2026");
    // encoded-строка PHC с параметрами 02 §6.2: t=2, m=19456, p=1
    CHECK(hash.rfind("$argon2id$v=19$m=19456,t=2,p=1$", 0) == 0);
    CHECK(PasswordHasher::verify(hash, "unit-test-password-2026"));
    CHECK_FALSE(PasswordHasher::verify(hash, "wrong-password"));
    // разные вызовы — разные соли (соль генерируется на каждый хэш)
    CHECK(PasswordHasher::hash("unit-test-password-2026") != hash);
}

TEST_CASE("LoginLimiter: 3 неудачи → блок 15 с, удвоение до потолка 15 мин")
{
    LoginLimiter limiter;
    const std::string login = "test.demo";
    int64_t t = 1'000'000;

    // до 3-й неудачи подряд вход разрешён (03 §4.1)
    limiter.onFailure(login, t);
    CHECK(limiter.blockedFor(login, t) == 0);
    limiter.onFailure(login, t + 1);
    CHECK(limiter.blockedFor(login, t + 1) == 0);
    // 3-я подряд — блок 15 с; внутри блока — оставшееся время
    limiter.onFailure(login, t + 2);
    CHECK(limiter.blockedFor(login, t + 2) == LoginLimiter::kInitialLockSec);
    CHECK(limiter.blockedFor(login, t + 2 + 14) == 1);
    // блок истёк в (t+2)+15 — с этой секунды вход снова разрешён
    t += 2 + LoginLimiter::kInitialLockSec;

    // после истечения блока каждая следующая неудача удваивает паузу:
    // 30 → 60 → 120 → 240 → 480 → 960 → потолок 900 с (15 минут)
    int64_t expect = LoginLimiter::kInitialLockSec * 2;
    while (true)
    {
        CHECK(limiter.blockedFor(login, t) == 0);
        limiter.onFailure(login, t);
        const int64_t capped =
            expect > LoginLimiter::kMaxLockSec ? LoginLimiter::kMaxLockSec
                                               : expect;
        CHECK(limiter.blockedFor(login, t) == capped);
        if (capped == LoginLimiter::kMaxLockSec)
            break;
        t += capped;
        expect *= 2;
    }
    // на потолке дальнейшие неудачи держат 900 с
    t += LoginLimiter::kMaxLockSec;
    CHECK(limiter.blockedFor(login, t) == 0);
    limiter.onFailure(login, t);
    CHECK(limiter.blockedFor(login, t) == LoginLimiter::kMaxLockSec);
}

TEST_CASE("LoginLimiter: успех сбрасывает счётчик; логины независимы")
{
    LoginLimiter limiter;
    const int64_t t = 5'000'000;
    limiter.onFailure("a.demo", t);
    limiter.onFailure("a.demo", t);
    limiter.onFailure("b.demo", t);
    CHECK(limiter.failures("a.demo") == 2);
    CHECK(limiter.failures("b.demo") == 1);
    CHECK(limiter.blockedFor("a.demo", t) == 0);
    CHECK(limiter.blockedFor("b.demo", t) == 0);
    // успех a.demo сбрасывает только a.demo
    limiter.onSuccess("a.demo");
    CHECK(limiter.failures("a.demo") == 0);
    CHECK(limiter.failures("b.demo") == 1);
}
