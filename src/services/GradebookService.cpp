#include "services/GradebookService.h"

#include <cmath>
#include <string>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Агрегат по дисциплинам (как kSummarySql GradesService — idx_grades_user:
// equality по user_id + GROUP BY) + works — число строк-работ дисциплины
// (счётчик «работ» зачётки). COUNT(g.id), а не COUNT(*): звёздочка в
// алиасной выборке drogon-sqlite читалась как пустой Field (works==0 при
// верных SUM — поймано юнит-тестом, см. журнал шага 3). control_type —
// из disciplines (бейдж 03 §4.4).
const std::string kGradebookSql =
    "SELECT g.discipline_id, d.code, d.title AS discipline, d.control_type, "
    "SUM(g.score) AS score_sum, SUM(g.max_score) AS max_sum, "
    "COUNT(g.id) AS works "
    "FROM grades g JOIN disciplines d ON d.id = g.discipline_id "
    "WHERE g.user_id = $1 "
    "GROUP BY g.discipline_id, d.code, d.title, d.control_type "
    "ORDER BY d.title";

// Процент, округлённый до целого (та же арифметика, что GradesService:
// llround, max<=0 → 0; целые, потому что jsoncpp дробные двойники пишет
// недетерминированно для демо — Э4).
int roundedPercent(int64_t scoreSum, int64_t maxSum)
{
    if (maxSum <= 0)
        return 0;
    return static_cast<int>(std::llround(100.0 * scoreSum / maxSum));
}

}  // namespace

void GradebookService::gradebook(DbClientPtr db, int64_t userId, ResultCb cb)
{
    *db << kGradebookSql << userId >>
        // Колбэк КОПИРУЕТСЯ в обе ветки цепочки (result/exception) — стреляет
        // ровно одна (урок Э2 шаг 1, OwnershipGuard.cpp).
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            int64_t totalScore = 0;
            int64_t totalMax = 0;
            int64_t totalWorks = 0;
            for (const auto& r : rows)
            {
                const int64_t scoreSum = r["score_sum"].as<int64_t>();
                const int64_t maxSum = r["max_sum"].as<int64_t>();
                const int64_t works = r["works"].as<int64_t>();
                totalScore += scoreSum;
                totalMax += maxSum;
                totalWorks += works;
                Json::Value d(Json::objectValue);
                d["discipline_id"] = r["discipline_id"].as<int64_t>();
                d["code"] = r["code"].as<std::string>();
                d["discipline"] = r["discipline"].as<std::string>();
                d["control_type"] = r["control_type"].as<std::string>();
                d["score_sum"] = scoreSum;
                d["max_sum"] = maxSum;
                d["percent"] = roundedPercent(scoreSum, maxSum);
                d["works"] = works;
                arr.append(std::move(d));
            }
            Json::Value body;
            if (rows.empty())
            {
                // Пустое состояние (Аня, staff): null, а не 0 — FE рисует
                // «оценки появятся после первой аттестации» (03 §4.4).
                body["average"] = Json::Value();
            }
            else
            {
                body["average"] = roundedPercent(totalScore, totalMax);
            }
            body["disciplines"] = std::move(arr);
            Json::Value counters(Json::objectValue);
            counters["disciplines"] = static_cast<int64_t>(rows.size());
            counters["works"] = totalWorks;
            body["counters"] = std::move(counters);
            cb(Result{Status::Ok, std::move(body)});
        } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "GradebookService::gradebook(" << userId
                      << "): " << e.base().what();
            cb(Result{Status::Error, Json::Value()});  // fail-closed → 500
        };
}

}  // namespace para
