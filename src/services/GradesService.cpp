#include "services/GradesService.h"

#include <cmath>
#include <string>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// Список: поля строки grades + код/название дисциплины JOIN'ом (карточки
// дисциплин 03 §4.9.3). Порядок — детерминированный: по дисциплине, внутри
// неё по времени выставления (баллы по модулям в карточке — в этом порядке).
// WHERE g.user_id = $1 — параметр (SQL только параметрами, 02 §6.4),
// idx_grades_user (user_id, discipline_id).
const std::string kListSql =
    "SELECT g.id, g.discipline_id, g.control_type, g.component, g.score, "
    "g.max_score, g.attempt, g.comment, g.graded_at, "
    "d.code, d.title AS discipline "
    "FROM grades g JOIN disciplines d ON d.id = g.discipline_id "
    "WHERE g.user_id = $1 ORDER BY d.title, g.graded_at, g.id";

// Сводка: агрегат по дисциплинам (idx_grades_user: equality по user_id +
// GROUP BY discipline_id — 02 §4.2). control_type дисциплины — из
// disciplines (бейдж типа контроля карточки, 03 §4.4); порог допуска
// считается в сервисе из max_sum — см. заголовок класса.
const std::string kSummarySql =
    "SELECT g.discipline_id, d.code, d.title AS discipline, d.control_type, "
    "SUM(g.score) AS score_sum, SUM(g.max_score) AS max_sum "
    "FROM grades g JOIN disciplines d ON d.id = g.discipline_id "
    "WHERE g.user_id = $1 "
    "GROUP BY g.discipline_id, d.code, d.title, d.control_type "
    "ORDER BY d.title";

// Процент набранных баллов, округлённый до целого (см. заголовок класса:
// jsoncpp дробные двойники пишет недетерминированно для демо, целые — нет).
int roundedPercent(int64_t scoreSum, int64_t maxSum)
{
    if (maxSum <= 0)
        return 0;
    return static_cast<int>(std::llround(100.0 * scoreSum / maxSum));
}

}  // namespace

void GradesService::list(DbClientPtr db, int64_t userId, ResultCb cb)
{
    *db << kListSql << userId >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
            {
                Json::Value g(Json::objectValue);
                g["id"] = r["id"].as<int64_t>();
                g["discipline_id"] = r["discipline_id"].as<int64_t>();
                g["code"] = r["code"].as<std::string>();
                g["discipline"] = r["discipline"].as<std::string>();
                g["control_type"] = r["control_type"].as<std::string>();
                g["component"] = r["component"].as<std::string>();
                g["score"] = r["score"].as<int64_t>();
                g["max_score"] = r["max_score"].as<int64_t>();
                g["attempt"] = r["attempt"].as<int64_t>();
                // Комментарий преподавателя — nullable (03 §4.9.3: выводится
                // при наличии, в кавычках); отсутствует → null.
                if (r["comment"].isNull())
                    g["comment"] = Json::Value();
                else
                    g["comment"] = r["comment"].as<std::string>();
                g["graded_at"] = r["graded_at"].as<std::string>();
                arr.append(std::move(g));
            }
            Json::Value body;
            body["grades"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        // Колбэк КОПИРУЕТСЯ в обе ветки цепочки (result/exception) — стреляет
        // ровно одна; `cb = std::move(cb)` в первой лямбде оставил бы второму
        // замыканию пустой std::function → bad_function_call на error-пути
        // (урок Э2 шаг 1, OwnershipGuard.cpp).
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "GradesService::list(" << userId
                      << "): " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void GradesService::summary(DbClientPtr db, int64_t userId, ResultCb cb)
{
    *db << kSummarySql << userId >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            int64_t totalScore = 0;
            int64_t totalMax = 0;
            for (const auto& r : rows)
            {
                const int64_t scoreSum = r["score_sum"].as<int64_t>();
                const int64_t maxSum = r["max_sum"].as<int64_t>();
                totalScore += scoreSum;
                totalMax += maxSum;
                Json::Value d(Json::objectValue);
                d["discipline_id"] = r["discipline_id"].as<int64_t>();
                d["code"] = r["code"].as<std::string>();
                d["discipline"] = r["discipline"].as<std::string>();
                d["control_type"] = r["control_type"].as<std::string>();
                d["score_sum"] = scoreSum;
                d["max_sum"] = maxSum;
                // Прогресс к допуску — процент набранных баллов (03 §2.8:
                // пороги цвета ≥60/40–59/<40 — целые проценты).
                d["percent"] = roundedPercent(scoreSum, maxSum);
                // «Что осталось» — сколько баллов до порога (демо-арифметика
                // 05 §3 Э4: у «Мировой экономики» 78/82 → 4).
                d["missing"] = maxSum > scoreSum ? maxSum - scoreSum : 0;
                arr.append(std::move(d));
            }
            Json::Value body;
            if (rows.empty())
            {
                // «Пустое» состояние сводки (Аня/стaff): null, а не 0 —
                // FE рисует «Оценки появятся после первой аттестации»
                // (03 §4.4), а не «средний 0».
                body["average"] = Json::Value();
            }
            else
            {
                body["average"] = roundedPercent(totalScore, totalMax);
            }
            body["disciplines"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "GradesService::summary(" << userId
                      << "): " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

}  // namespace para
