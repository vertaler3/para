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
};

}  // namespace para
