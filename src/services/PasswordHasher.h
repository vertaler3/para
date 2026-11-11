#pragma once

#include <string>

namespace para {

// Обёртка над libargon2 (phc-winner-argon2, CC0/Apache-2.0 — 02 §2.2,
// docs/decisions.md Р-7). Параметры — 02 §6.2 (OWASP): Argon2id,
// t=2, m=19456 КБ, p=1, тег 32 байта. MD5/SHA256 из Drogon Utilities.h
// для паролей запрещены правилом проекта.
class PasswordHasher
{
  public:
    // Хэшировать пароль (соль генерирует библиотека; наружу — только
    // PHC-encoded-строка $argon2id$v=19$m=19456,t=2,p=1$…).
    static std::string hash(const std::string& password);

    // Проверить пароль против encoded-строки (argon2id_verify —
    // constant-time сравнение внутри библиотеки, 02 §6.2).
    static bool verify(const std::string& encodedHash,
                       const std::string& password);

    // Фиктивная проверка той же стоимости (Э8 шаг 4, находка
    // Э1-timing-оракул): тот же код пути, что и verify() — argon2id_verify
    // против PHC-строки с ТЕМИ ЖЕ параметрами (t=2, m=19456, p=1) и
    // солью-заглушкой. Результат заведомо false: вызывается контроллером
    // входа на ветке «логин не найден», чтобы время ответа не различало
    // «нет логина» и «неверный пароль» (02 §6.2/§6.4).
    static bool verifyDummy(const std::string& password);
};

}  // namespace para
