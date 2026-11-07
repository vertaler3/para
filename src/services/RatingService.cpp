#include "services/RatingService.h"

#include <cmath>
#include <string>

#include <drogon/drogon.h>

#include "utils/NameMask.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Студенты (+факультет) со средним: LEFT JOIN grades — студенты без оценок
// остаются в списке (score_sum/max_sum NULL → average null, конец списка).
// Факультет-фильтр — через «пустая строка = фильтра нет» (фиксированное
// число параметров, урок Э5: порядок $N = порядку биндинга).
// Сортировка: сначала оценённые по среднему DESC (NULL-математика SQLite
// ставит NULL последними при DESC), затем детерминированно по user_id.
const std::string kRatingSql =
    "SELECT u.id AS user_id, p.fio, p.faculty, "
    "SUM(g.score) AS score_sum, SUM(g.max_score) AS max_sum "
    "FROM users u JOIN profiles p ON p.user_id = u.id "
    "LEFT JOIN grades g ON g.user_id = u.id "
    "WHERE u.role = 'student' AND ($1 = '' OR p.faculty = $1) "
    "GROUP BY u.id, p.fio, p.faculty "
    "ORDER BY (SUM(g.max_score) IS NULL) ASC, "
    "(100.0 * SUM(g.score) / SUM(g.max_score)) DESC, u.id ASC";

// Средний — взвешенный процент Σscore/Σmax×100, целое (арифметика
// GradesService::summary Э4 / GradebookService); NULL → null.
Json::Value averageOf(const drogon::orm::Row& r)
{
    if (r["max_sum"].isNull())
        return Json::Value();  // оценок нет
    const int64_t maxSum = r["max_sum"].as<int64_t>();
    if (maxSum <= 0)
        return Json::Value();
    const int64_t scoreSum = r["score_sum"].as<int64_t>();
    return Json::Value(
        static_cast<int64_t>(std::llround(100.0 * scoreSum / maxSum)));
}

}  // namespace

void RatingService::rating(DbClientPtr db,
                           const PeopleService::Requester& requester,
                           const std::optional<std::string>& faculty,
                           int maxRows,
                           ResultCb cb)
{
    // Факультет рейтинга: у студента — ТОЛЬКО свой (из БД по сессии, 02
    // §6.3); у staff — query faculty или все (пустая строка = без фильтра).
    std::string facultyArg;
    std::string facultyTitle;
    if (requester.role == "student")
    {
        if (!requester.faculty)
        {
            // Студент без профиля не бывает (profiles NOT NULL, 0001) —
            // fail-closed: не рейтинг «всех», а ошибка.
            cb(Result{Status::Error, Json::Value(), ""});
            return;
        }
        facultyArg = *requester.faculty;
        facultyTitle = *requester.faculty;
    }
    else if (faculty)
    {
        facultyArg = *faculty;
        facultyTitle = *faculty;
    }

    *db << kRatingSql << facultyArg >>
        // Колбэк КОПИРУЕТСЯ в обе ветки цепочки (урок Э2 шаг 1).
        [cb, requester, facultyTitle, maxRows](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            int64_t rank = 0;
            for (const auto& r : rows)
            {
                // limit — после сортировки (SQL без LIMIT); 0 = без
                // ограничения (контроллер не передал ?limit=).
                if (maxRows > 0 &&
                    static_cast<int>(arr.size()) >= maxRows)
                    break;
                ++rank;
                const int64_t userId = r["user_id"].as<int64_t>();
                const bool self = userId == requester.userId;
                Json::Value line(Json::objectValue);
                line["rank"] = rank;
                line["user_id"] = userId;
                // Маскирование НА СЕРВЕРЕ: полное имя чужих в ответ не
                // попадает (02 §5.2) — только «И. Фамилия» (NameMask).
                line["fio"] = self ? r["fio"].as<std::string>()
                                   : maskFullName(r["fio"].as<std::string>());
                line["average"] = averageOf(r);
                line["self"] = self;
                arr.append(std::move(line));
            }
            Json::Value body;
            body["faculty"] =
                facultyTitle.empty() ? Json::Value() : Json::Value(facultyTitle);
            body["rating"] = std::move(arr);
            body["masked"] = true;  // контракт 02 §5.2 — маскирование на сервере
            cb(Result{Status::Ok, std::move(body), ""});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "RatingService::rating: " << e.base().what();
            cb(Result{Status::Error, Json::Value(), ""});  // fail-closed → 500
        };
}

}  // namespace para
