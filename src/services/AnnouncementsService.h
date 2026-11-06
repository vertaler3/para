#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/SessionService.h"

namespace para {

// Объявления деканата (Э6 шаг 4; 01 §4.5: объявления push-ем НЕ отправляются
// — живут в ленте, в бейдже непрочитанных и в дайджесте «что нового»;
// 02 §5.2: GET /v1/announcements — «по аудитории своей группы/курса»,
// POST /v1/staff/announcements — 201 · 403 (StaffFilter); таблица
// announcements миграции 0003).
//
// Аудитория — колонка audience TEXT: 'all' | JSON-массив целей. Формат цели
// (канон сервиса): строка «group:<id>» или «course:<id>»; пример значения
// колонки: ["group:1","course:4"]. Видимость: audience='all' — всем;
// иначе массив содержит «group:<gid>» (группа студента) или
// «course:<course>» (курс студента из профиля); staff (профиля нет, Р-6)
// видит все — рабочий инструмент.
class AnnouncementsService
{
  public:
    struct Audience
    {
        bool all = true;
        // (kind, id): kind — "group" | "course".
        std::vector<std::pair<std::string, int64_t>> targets;
    };

    // "all" | JSON-массив строк "group:<id>"/"course:<id>" (id > 0);
    // иначе false → контроллер 400.
    static bool parseAudience(const std::string& audienceJson, Audience& out);
    // Сериализация в TEXT колонки (канон: массив JSON-строк).
    static std::string serializeAudience(const Audience& a);

    struct Result
    {
        bool ok = false;  // false — ошибка БД → 500
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // Лента объявлений, видимых сессии (группа/курс из профиля; staff — все),
    // новые сверху; cursor — id последнего загруженного (id < cursor),
    // ответ {announcements: […до 20], next_cursor: id|null}.
    static void list(drogon::orm::DbClientPtr db,
                     const SessionInfo& session,
                     std::optional<int64_t> cursor,
                     ResultCb cb);

    // Публикация (staff): INSERT без push/уведомлений (01 §4.5); ответ —
    // объект объявления (201).
    static void publish(drogon::orm::DbClientPtr db,
                        int64_t staffId,
                        const std::string& title,
                        const std::string& body,
                        const Audience& audience,
                        ResultCb cb);
};

}  // namespace para
