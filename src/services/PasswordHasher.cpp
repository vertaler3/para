#include "services/PasswordHasher.h"

#include <array>
#include <stdexcept>
#include <cstring>

#include <argon2.h>
#include <openssl/rand.h>

namespace {

// PHC-строка для verifyDummy: те же параметры, что у hash() (02 §6.2:
// Argon2id, t=2, m=19456 КБ, p=1), соль-заглушка «para-dummy-salt0»
// (16 байт, base64 cGFyYS1kdW1teS1zYWx0MA). Не секрет и не чей-то пароль:
// это хэш непроверяемого паддинга «para-timing-pad», единственная задача
// которого — «сжечь» то же время, что и реальная проверка (Э8 шаг 4,
// находка Э1-timing-оракул; приём OWASP «dummy verify»).
constexpr char kDummyEncoded[] =
    "$argon2id$v=19$m=19456,t=2,p=1$cGFyYS1kdW1teS1zYWx0MA$"
    "PvLBWTa1ezGmo3ucQF7Fs1nrCNQ6ccFxvli9H5/GntM";

}  // namespace

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

bool PasswordHasher::verifyDummy(const std::string& password)
{
    // Тот же путь кода, что и verify(): парсинг PHC-строки, argon2id с теми
    // же параметрами, constant-time сравнение. Вернёт false — пароль не
    // совпадает с паддингом; важна не истинность, а равная стоимость.
    return argon2id_verify(kDummyEncoded, password.data(),
                           password.size()) == ARGON2_OK;
}

}  // namespace para
