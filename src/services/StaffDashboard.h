#pragma once

#include <functional>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Рабочий стол сотрудника (Э6 шаг 4; 02 §5.2: GET /v1/staff/dashboard —
// «счётчики очереди» · 403 отдаёт StaffFilter). SQL здесь (AGENTS.md §5.5).
// Ответ:
//   {"queue": {"submitted": N, "accepted": N, "in_progress": N,
//              "need_info": N, "ready": N},        ← dashboardMarker: queue
//    "recent_changes": [ события 02 §5.2 с prev_* — последние 5 изменений
//                        расписания (status != 'normal') ]}
// Активные статусы очереди — словарь CHECK 0003 (draft/терминальные в
// «очередь» не входят: не требуют действия сотрудника).
class StaffDashboard
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД → 500
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    static void build(drogon::orm::DbClientPtr db, ResultCb cb);
};

}  // namespace para
