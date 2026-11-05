#include "services/SettingsService.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// Допустимые темы — токены 03 §2 (системная по prefers-color-scheme + ручной
// переключатель); дефолт 'system' — из DDL миграции 0004.
bool isValidTheme(const std::string& theme)
{
    return theme == "light" || theme == "dark" || theme == "system";
}

// Строка-поле тела обязана содержать валидный JSON (02 §5.2).
bool isValidJsonString(const std::string& s)
{
    Json::Value parsed;
    Json::Reader reader;  // jsoncpp 1.y: Reader допускает любой JSON
    return reader.parse(s, parsed);
}

}  // namespace

SettingsRecord SettingsService::defaults()
{
    // Ровно значения DEFAULT из миграции 0004 (02 §4.1): тумблеры включены,
    // тихие часы 23:00–07:00, тема системная, чеклист онбординга пуст.
    return SettingsRecord{};
}

void SettingsService::load(drogon::orm::DbClientPtr db,
                           int64_t userId,
                           std::function<void(SettingsRecord)> cb)
{
    *db << "SELECT notify_schedule, notify_deadlines, notify_applications, "
           "quiet_hours_json, theme, onboarding_json "
           "FROM settings WHERE user_id = $1"
        << userId >>
        [cb = std::move(cb), userId](const Result& rows) {
            if (rows.empty())
            {
                // Строки нет — дефолты по 02 §5.2; запись создаёт первый PUT.
                auto rec = defaults();
                rec.userId = userId;
                cb(std::move(rec));
                return;
            }
            SettingsRecord rec;
            rec.userId = userId;
            rec.notifySchedule = rows[0]["notify_schedule"].as<int>() != 0;
            rec.notifyDeadlines = rows[0]["notify_deadlines"].as<int>() != 0;
            rec.notifyApplications =
                rows[0]["notify_applications"].as<int>() != 0;
            rec.quietHoursJson = rows[0]["quiet_hours_json"].as<std::string>();
            rec.theme = rows[0]["theme"].as<std::string>();
            rec.onboardingJson = rows[0]["onboarding_json"].as<std::string>();
            cb(std::move(rec));
        } >>
        // cb копируется в обе ветки (result/exception): стреляет ровно одна;
        // при move во вторую лямбду она получила бы пустой std::function →
        // std::bad_function_call на ошибке БД (см. OwnershipGuard, Э2 шаг 1).
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "SettingsService::load(" << userId
                      << "): " << e.base().what();
            auto rec = defaults();
            rec.userId = userId;
            cb(std::move(rec));  // fail-closed: дефолты вместо сбоя
        };
}

void SettingsService::save(drogon::orm::DbClientPtr db,
                           const SettingsRecord& rec,
                           std::function<void(bool ok)> cb)
{
    // UPSERT по первичному ключу user_id: строка ровно одного пользователя —
    // того, чья сессия (user_id из тела запроса сюда не доходит).
    *db << "INSERT INTO settings (user_id, notify_schedule, notify_deadlines, "
           "notify_applications, quiet_hours_json, theme, onboarding_json) "
           "VALUES ($1, $2, $3, $4, $5, $6, $7) "
           "ON CONFLICT(user_id) DO UPDATE SET "
           "notify_schedule = excluded.notify_schedule, "
           "notify_deadlines = excluded.notify_deadlines, "
           "notify_applications = excluded.notify_applications, "
           "quiet_hours_json = excluded.quiet_hours_json, "
           "theme = excluded.theme, "
           "onboarding_json = excluded.onboarding_json"
        << rec.userId << (rec.notifySchedule ? 1 : 0)
        << (rec.notifyDeadlines ? 1 : 0) << (rec.notifyApplications ? 1 : 0)
        << rec.quietHoursJson << rec.theme << rec.onboardingJson >>
        [cb = std::move(cb)](const Result&) { cb(true); } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "SettingsService::save: " << e.base().what();
            cb(false);  // 500 без деталей (02 §5.1)
        };
}

SettingsService::PatchResult SettingsService::applyPatch(
    const SettingsRecord& base, const Json::Value& body, SettingsRecord& out)
{
    out = base;
    PatchResult ok;
    ok.ok = true;

    // user_id в теле игнорируется намеренно (изоляция данных 02 §6.3):
    // значение не читается вовсе — гейт проверяет подмену.

    if (body.isMember("notify_schedule"))
    {
        if (!body["notify_schedule"].isBool())
            return {false, "bad_request",
                    "notify_schedule — ожидается true/false"};
        out.notifySchedule = body["notify_schedule"].asBool();
    }
    if (body.isMember("notify_deadlines"))
    {
        if (!body["notify_deadlines"].isBool())
            return {false, "bad_request",
                    "notify_deadlines — ожидается true/false"};
        out.notifyDeadlines = body["notify_deadlines"].asBool();
    }
    if (body.isMember("notify_applications"))
    {
        if (!body["notify_applications"].isBool())
            return {false, "bad_request",
                    "notify_applications — ожидается true/false"};
        out.notifyApplications = body["notify_applications"].asBool();
    }
    if (body.isMember("theme"))
    {
        if (!body["theme"].isString() ||
            !isValidTheme(body["theme"].asString()))
            return {false, "bad_request",
                    "theme — допустимо light, dark или system"};
        out.theme = body["theme"].asString();
    }
    if (body.isMember("quiet_hours_json"))
    {
        if (!body["quiet_hours_json"].isString() ||
            !isValidJsonString(body["quiet_hours_json"].asString()))
            return {false, "bad_request",
                    "quiet_hours_json — ожидается строка с валидным JSON"};
        out.quietHoursJson = body["quiet_hours_json"].asString();
    }
    if (body.isMember("onboarding_json"))
    {
        if (!body["onboarding_json"].isString() ||
            !isValidJsonString(body["onboarding_json"].asString()))
            return {false, "bad_request",
                    "onboarding_json — ожидается строка с валидным JSON"};
        out.onboardingJson = body["onboarding_json"].asString();
    }
    return ok;
}

void SettingsService::toJson(const SettingsRecord& rec, Json::Value& out)
{
    out["notify_schedule"] = rec.notifySchedule;
    out["notify_deadlines"] = rec.notifyDeadlines;
    out["notify_applications"] = rec.notifyApplications;
    // *_json отдаются как вложенные JSON-значения, а не строки:
    // Json::Reader.parse уже проверил валидность при записи (applyPatch) и
    // в БД лежит только валидный JSON (DEFAULT-строки миграции 0004 тоже).
    Json::Value quiet;
    Json::Reader().parse(rec.quietHoursJson, quiet);
    out["quiet_hours"] = quiet;
    Json::Value onboarding;
    Json::Reader().parse(rec.onboardingJson, onboarding);
    out["onboarding"] = onboarding;
    out["theme"] = rec.theme;
}

}  // namespace para
