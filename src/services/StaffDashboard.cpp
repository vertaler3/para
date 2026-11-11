#include "services/StaffDashboard.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Очередь — активные статусы (словарь CHECK миграции 0003): draft подаёт
// сам студент, issued/rejected/cancelled — завершены.
const std::string kQueueSql =
    "SELECT status, count(*) AS n FROM applications "
    "WHERE status IN ('submitted','accepted','in_progress','need_info','ready')"
    " GROUP BY status";

// Последние изменения расписания (свежие события — «было → стало»):
// idx_schedule_group_status не применим (по всем группам) — таблица
// мала (демо-контур), сортировка по changed_at DESC.
const std::string kRecentSql =
    "SELECT e.id, e.group_id, e.discipline_id, e.teacher_id, e.room, "
    "       e.kind, e.starts_at, e.ends_at, e.status, e.prev_starts_at, "
    "       e.prev_room, e.prev_teacher_id, e.changed_at, "
    "       d.title AS discipline_title, d.code AS discipline_code "
    "FROM schedule_events e JOIN disciplines d ON d.id = e.discipline_id "
    "WHERE e.status != 'normal' "
    "ORDER BY e.changed_at DESC, e.id DESC LIMIT 5";

Json::Value eventJsonOf(const Row& r)
{
    // Копия eventJson ScheduleService.cpp (формат событий 02 §5.2).
    Json::Value e(Json::objectValue);
    e["id"] = r["id"].as<int64_t>();
    e["discipline"] = r["discipline_title"].as<std::string>();
    e["code"] = r["discipline_code"].as<std::string>();
    e["kind"] = r["kind"].as<std::string>();
    e["room"] = r["room"].as<std::string>();
    e["starts_at"] = r["starts_at"].as<std::string>();
    e["ends_at"] = r["ends_at"].as<std::string>();
    e["status"] = r["status"].as<std::string>();
    e["prev_room"] = r["prev_room"].isNull()
                         ? Json::Value()
                         : Json::Value(r["prev_room"].as<std::string>());
    e["prev_starts_at"] =
        r["prev_starts_at"].isNull()
            ? Json::Value()
            : Json::Value(r["prev_starts_at"].as<std::string>());
    e["prev_teacher_id"] =
        r["prev_teacher_id"].isNull()
            ? Json::Value()
            : Json::Value(r["prev_teacher_id"].as<int64_t>());
    e["changed_at"] = r["changed_at"].isNull()
                          ? Json::Value()
                          : Json::Value(r["changed_at"].as<std::string>());
    return e;
}

}  // namespace

void StaffDashboard::build(DbClientPtr db, ResultCb cb)
{
    if (!db)
    {
        cb(Result{});
        return;
    }
    *db << kQueueSql >>
        // cb КОПИРУЕТСЯ во все ветки обеих цепочек (стреляет ровно одна);
        // move во внутренней лямбде оставил бы error-ветке пустой
        // std::function → bad_function_call (урок Э2, Э8 шаг 5).
        [db, cb](const drogon::orm::Result& rows) mutable {
            Json::Value queue(Json::objectValue);
            for (const char* s : {"submitted", "accepted", "in_progress",
                                  "need_info", "ready"})
                queue[s] = 0;  // отсутствующий статус — 0
            for (const auto& r : rows)
                queue[r["status"].as<std::string>()] =
                    static_cast<Json::Int64>(r["n"].as<int64_t>());

            // Свежие изменения расписания — вторая фаза.
            *db << kRecentSql >>
                [cb, queue = std::move(queue)](
                    const drogon::orm::Result& recent) mutable {
                    Json::Value changes(Json::arrayValue);
                    for (const auto& r : recent)
                        changes.append(eventJsonOf(r));
                    Json::Value body(Json::objectValue);
                    body["queue"] = std::move(queue);
                    body["recent_changes"] = std::move(changes);
                    cb(Result{true, std::move(body)});
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "StaffDashboard: recent: " << e.base().what();
                    cb(Result{});
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "StaffDashboard: queue: " << e.base().what();
            cb(Result{});
        };
}

}  // namespace para
