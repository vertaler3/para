#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Настройки уведомлений и оформления — таблица settings (миграция 0004;
// 02 §4.1, §5.2: GET/PUT /v1/notifications/settings). Весь SQL —
// параметризованный и живёт здесь, не в контроллерах (AGENTS.md §5.5,
// 02 §6.4). user_id записи всегда берётся из сессии в контроллере;
// user_id из тела PUT игнорируется (изоляция данных 02 §6.3).
struct SettingsRecord
{
    int64_t userId = 0;
    bool notifySchedule = true;
    bool notifyDeadlines = true;
    bool notifyApplications = true;
    std::string quietHoursJson = R"({"from":"23:00","to":"07:00"})";
    std::string theme = "system";  // токены 03 §2: light | dark | system
    std::string onboardingJson = "[]";  // состояние чеклиста онбординга С5
};

class SettingsService
{
  public:
    // Дефолты при отсутствии строки — значения DEFAULT из DDL миграции 0004
    // (все тумблеры включены, тихие часы 23:00–07:00, тема системная,
    // онбординг-чеклист не начат).
    static SettingsRecord defaults();

    // Строка настроек по user_id; cb(defaults()) — строки нет (ленивое
    // создание: строка появляется при первом PUT).
    static void load(drogon::orm::DbClientPtr db,
                     int64_t userId,
                     std::function<void(SettingsRecord)> cb);

    // UPSERT строки настроек (INSERT … ON CONFLICT(user_id) DO UPDATE —
    // затрагивает ровно одну строку: user_id записи = rec.userId, из
    // сессии). cb(false) — ошибка БД (контроллер отвечает 500).
    static void save(drogon::orm::DbClientPtr db,
                     const SettingsRecord& rec,
                     std::function<void(bool ok)> cb);

    // Применить тело PUT к текущей записи (частичное обновление: поле без
    // значения в теле не меняется — так закрывается шаг онбординга С5
    // одиночным {"onboarding_json": …}). user_id из тела игнорируется:
    // userId записи приходит только из сессии (изоляция 02 §6.3).
    // theme — только light/dark/system (токены 03 §2); quiet_hours_json и
    // onboarding_json — строки с валидным JSON (02 §5.2).
    // Возвращает true при успехе; иначе errorCode/errorMessage для ответа
    // 400 в формате 02 §5.1 (вложенный error.code/message).
    struct PatchResult
    {
        bool ok = false;
        std::string errorCode;
        std::string errorMessage;
    };
    static PatchResult applyPatch(const SettingsRecord& base,
                                  const Json::Value& body,
                                  SettingsRecord& out);

    // JSON-представление записи для ответов GET/PUT (02 §5.2: три тумблера
    // + тихие часы + тема; *_json отдаются как вложенные JSON-значения).
    static void toJson(const SettingsRecord& rec, Json::Value& out);
};

}  // namespace para
