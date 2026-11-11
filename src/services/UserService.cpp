#include "services/UserService.h"

#include <ctime>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {

void UserService::findByLogin(
    drogon::orm::DbClientPtr db,
    const std::string& login,
    std::function<void(std::optional<UserRecord>)> cb)
{
    *db << "SELECT id, login, role, password_hash FROM users WHERE login = $1"
        << login >>
        // cb КОПИРУЕТСЯ в обе ветки цепочки (result/exception) — стреляет
        // ровно одна; move в первой лямбде оставил бы второй замыканию
        // пустой std::function → std::bad_function_call на error-пути
        // (урок Э2 шаг 1; латентный аналог, найден и исправлен Э8 шаг 5).
        [cb](const Result& rows) {
            if (rows.empty())
            {
                cb(std::nullopt);
                return;
            }
            UserRecord record;
            record.id = rows[0]["id"].as<int64_t>();
            record.login = rows[0]["login"].as<std::string>();
            record.role = rows[0]["role"].as<std::string>();
            record.passwordHash = rows[0]["password_hash"].as<std::string>();
            cb(std::move(record));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "UserService::findByLogin: " << e.base().what();
            cb(std::nullopt);
        };
}

void UserService::loadProfile(
    drogon::orm::DbClientPtr db,
    int64_t userId,
    std::function<void(std::optional<Json::Value>)> cb)
{
    *db << "SELECT p.fio, p.faculty, p.program, p.course, "
           "g.name AS group_name, p.student_card_no "
           "FROM profiles p JOIN groups g ON g.id = p.group_id "
           "WHERE p.user_id = $1"
        << userId >>
        // Копия cb в обе ветки — урок Э2 (см. findByLogin выше).
        [cb](const Result& rows) {
            if (rows.empty())
            {
                cb(std::nullopt);  // staff без профиля (02 §4.1)
                return;
            }
            const auto& row = rows[0];
            Json::Value profile;
            profile["fio"] = row["fio"].as<std::string>();
            profile["faculty"] = row["faculty"].as<std::string>();
            profile["program"] = row["program"].as<std::string>();
            profile["course"] = row["course"].as<int>();
            profile["group"] = row["group_name"].as<std::string>();
            profile["student_card_no"] = row["student_card_no"].as<std::string>();
            cb(std::move(profile));
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "UserService::loadProfile: " << e.base().what();
            cb(std::nullopt);
        };
}

void UserService::mirrorLimiterState(drogon::orm::DbClientPtr db,
                                     const std::string& login,
                                     int failures,
                                     int64_t lockedUntilSec)
{
    // locked_until — колонка TEXT (0001_identity.sql): unix-сек → ISO-UTC,
    // тот же формат строк времени, что у starts_at/ends_at (сравнения и
    // сортировки лексикографически корректны); 0 → NULL — блокировки нет.
    // Порядок $N в тексте = порядку биндинга (урок Э5 шаг 2).
    std::string lockedIso;
    if (lockedUntilSec > 0)
    {
        const std::time_t tt = static_cast<std::time_t>(lockedUntilSec);
        std::tm tm{};
        gmtime_r(&tt, &tm);
        char buf[24];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        lockedIso = buf;
    }
    *db << "UPDATE users SET failed_attempts = $1, locked_until = "
           "CASE WHEN $2 = '' THEN NULL ELSE $2 END WHERE login = $3"
        << failures << lockedIso << login >>
        [](const Result&) {} >>
        [](const DrogonDbException& e) {
            LOG_ERROR << "UserService::mirrorLimiterState: "
                      << e.base().what();
        };
}

void UserService::resetLimiterMirror(drogon::orm::DbClientPtr db,
                                     const std::string& login)
{
    *db << "UPDATE users SET failed_attempts = 0, locked_until = NULL "
           "WHERE login = $1"
        << login >>
        [](const Result&) {} >>
        [](const DrogonDbException& e) {
            LOG_ERROR << "UserService::resetLimiterMirror: "
                      << e.base().what();
        };
}

}  // namespace para
