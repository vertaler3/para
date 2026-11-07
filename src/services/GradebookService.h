#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para
{

// Зачётная книжка (5.15; 02 §5.2: GET /v1/gradebook — «дисциплины с итогами,
// средний балл, счётчики»). Расчёт из существующих grades (Р-10: без новых
// сущностей), только СВОИ строки — user_id из сессии (02 §6.3), guard не
// нужен (нет path-id). Весь SQL — параметризованный и живёт здесь
// (AGENTS.md §5.5), индекс idx_grades_user (user_id, discipline_id).
//
// ИТОГ дисциплины — сумма score/max по всем строкам дисциплины (постановка
// шага; фикстуры не содержат пересдач — при появлении attempt≥2 строки
// суммируются так же, поведение фиксирует юнит-тест). Средний балл — тот же
// взвешенный процент Σscore/Σmax×100, ЦЕЛОЕ (стиль GradesService::summary
// Э4: jsoncpp дробные двойники пишет недетерминированно; «Мировая
// экономика» 78/82 → percent 95, average Марка 83). Пустое состояние (Аня,
// staff) — average null и пустые списки, а не нули («оценки появятся после
// первой аттестации», 03 §4.4).
class GradebookService
{
  public:
    // Ok — 200; Error — ошибка БД → 500 (fail-closed, не пустая книжка).
    enum class Status
    {
        Ok,
        Error
    };
    struct Result
    {
        Status status = Status::Ok;
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/gradebook → {"average", "disciplines": [{discipline_id, code,
    // discipline, control_type, score_sum, max_sum, percent}], "counters":
    // {"disciplines", "works"}} — works = число строк grades (работ),
    // disciplines = число дисциплин с итогами; порядок — по названию.
    static void gradebook(drogon::orm::DbClientPtr db,
                          int64_t userId,
                          ResultCb cb);
};

}  // namespace para
