#include "services/PasswordHasher.h"

#include <array>
#include <stdexcept>
#include <cstring>

#include <argon2.h>
#include <openssl/rand.h>

namespace para {

std::string PasswordHasher::hash(const std::string& password)
{
    // Соль генерируем сами (16 байт CSPRNG); в БД уходит только PHC-encoded
    // строка — соль и параметры зашиты в неё (02 §6.2).
    std::array<unsigned char, 16> salt{};
    if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1)
    {
        throw std::runtime_error("PasswordHasher: RAND_bytes failed");
    }
    // Параметры 02 §6.2 (OWASP): t=2, m=19456 КБ, p=1, тег 32 байта.
    std::string encoded(128, '\0');
    const int rc = argon2id_hash_encoded(
        /*t=*/2,
        /*m=*/19456,
        /*p=*/1,
        password.data(),
        password.size(),
        salt.data(),
        salt.size(),
        /*hashlen=*/32,
        encoded.data(),
        encoded.size());
    if (rc != ARGON2_OK)
    {
        throw std::runtime_error(std::string("PasswordHasher: argon2id: ") +
                                 argon2_error_message(rc));
    }
    encoded.resize(std::strlen(encoded.c_str()));
    return encoded;
}

bool PasswordHasher::verify(const std::string& encodedHash,
                            const std::string& password)
{
    return argon2id_verify(encodedHash.c_str(), password.data(),
                           password.size()) == ARGON2_OK;
}

}  // namespace para
