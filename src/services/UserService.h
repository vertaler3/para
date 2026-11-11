#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Доступ к учётным записям и профилям (таблицы users/profiles, 02 §4.1).
// Весь SQL — параметризованный и живёт здесь, не в контроллерах
// (AGENTS.md §5.5, 02 §6.4).
struct UserRecord
{
    int64_t id = 0;
    std::string login;
    std::string role;
    std::string passwordHash;  // argon2id-encoded
};

class UserService
{
  public:
    // Учётная запись по логину (для проверки пароля при входе);
    // cb(std::nullopt) — логин не найден.
    static void findByLogin(drogon::orm::DbClientPtr db,
                            const std::string& login,
                            std::function<void(std::optional<UserRecord>)> cb);

    // Профиль пользователя для GET /v1/me: fio/faculty/program/course/
    // group/student_card_no; cb(std::nullopt) — профиля нет (staff).
    static void loadProfile(
        drogon::orm::DbClientPtr db,
        int64_t userId,
        std::function<void(std::optional<Json::Value>)> cb);

    // Полное зеркало лимитера в users (02 §4.1/§6.4; Э8 шаг 7 — находка
    // Э1 «неполное зеркало»: раньше писался только failed_attempts,
    // locked_until не писал никто, а успешный вход не сбрасывал зеркала).
    // Неудача пишет failed_attempts И locked_until (unix-сек → ISO-UTC,
    // 0 → NULL — блокировки нет); успех — resetLimiterMirror (оба поля в
    // исходные). best-effort: ошибки пишутся в лог, ход входа не ломают.
    // Зеркало — СОСТОЯНИЕ ДЛЯ НАБЛЮДЕНИЯ, а не источник лимитера:
    // blockedFor читает память процесса; сохранение блокировок между
    // рестартами —
    // Р-26 (сознательно отложено, там же точная задача).
    static void mirrorLimiterState(drogon::orm::DbClientPtr db,
                                   const std::string& login,
                                   int failures,
                                   int64_t lockedUntilSec);

    // Сброс зеркала лимитера после успешного входа (см. выше).
    static void resetLimiterMirror(drogon::orm::DbClientPtr db,
                                   const std::string& login);
};

}  // namespace para
