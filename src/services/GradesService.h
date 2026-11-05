#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// Успеваемость (Э4 шаг 1; 02 §5.2: GET /v1/grades — «свои оценки по
// дисциплинам», GET /v1/grades/summary — «сводка семестра: средний балл,
// прогресс, „что осталось“»; таблицы grades/disciplines — миграция 0002).
// Весь SQL — параметризованный и живёт здесь, не в контроллерах (AGENTS.md
// §5.5, 02 §6.4). Выборки по индексу idx_grades_user (user_id, discipline_id)
// — 02 §4.2: «Виджет „Учёба“ (С4) — оценки пользователя по дисциплинам»:
// WHERE user_id = $1 + GROUP BY discipline_id — один индекс на запрос.
//
// user_id — только из сессии (контроллер берёт его из атрибута LoginFilter,
// 02 §6.3): у этих роутов нет path-id — OwnershipGuard не применим (правило
// «контроллер с path-id обязан вызвать guard» — про эндпоинты с {id}).
//
// Семантика сводки — демо-арифметика 05 §3 Э4 / 03 §4.9.3 («30+30+18=78 при
// пороге 82 — до допуска не хватает 4»):
//   порог допуска дисциплины = Σ max_score её компонентов, у которых есть
//   оценки (фикстуры d1: 30+30+22=82); прогресс = Σ score / Σ max_score;
//   «что осталось» (missing) = max(0, Σ max_score − Σ score) — у d1 это 4;
//   строки дисциплины суммируются как есть — пересдача (attempt ≥ 2) по
//   05 §3 Э4 показывается отдельной строкой списка, правило пересчёта итога
//   после пересдачи — ветка этой строки Э4, не шага 1;
//   средний балл (average) — взвешенный процент набранных баллов по всем
//   дисциплинам с оценками (Σ score / Σ max_score × 100), округлён до целого.
//   Целые — сознательно: jsoncpp с долями пишет репрезентацию двойника
//   целиком («83.2» → «83.200000000000003» — проверено в образе v1.9.14),
//   целые числа сериализуются детерминированно, подписи tnum в 03 (§2.8,
//   прогресс-бары «78/100») — тоже целые проценты.
class GradesService
{
  public:
    struct Result
    {
        bool ok = false;  // false — ошибка БД → контроллер отвечает 500
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/grades → {"grades": [...]}: свои записи оценок (user_id из
    // сессии), по дисциплинам, внутри дисциплины — по времени; код/название
    // дисциплины JOIN'ом. Оценок нет — 200 с пустым grades (список пуст и
    // «пустая» сводка — разные представления одного состояния, С5.6).
    static void list(drogon::orm::DbClientPtr db, int64_t userId, ResultCb cb);

    // GET /v1/grades/summary → {"average": int|null, "disciplines": [...]}.
    // Оценок нет (Аня — «оценки появятся после первой аттестации», 03 §4.4;
    // staff без оценок — так же) — 200 с average=null и пустым disciplines:
    // «пустое» состояние сводки, компактно сериализуется как
    // {"average":null,"disciplines":[]}.
    static void summary(drogon::orm::DbClientPtr db, int64_t userId,
                        ResultCb cb);

    // GET /v1/grades/{id} — одна запись. Владение проверяет КОНТРОЛЛЕР через
    // OwnershipGuard::check(Table::Grades) ДО вызова этого метода (правило
    // код-ревью в шапке OwnershipGuard.h; 02 §6.3 п.3 — guard различает
    // NotFound/Forbidden). Выборка здесь — по PK БЕЗ условия user_id:
    // дублировать проверку владения SQL-условием мимо guard'а нельзя
    // (постановка Э4 шаг 2; guard — единственная точка решения о доступе).
    // Found — строка есть (body — объект той же формы, что в list);
    // NotFound — строки нет (после Own guard'а — удаление в гонке; контроллер
    // отвечает 404); Error — ошибка БД → 500, fail-closed.
    enum class One
    {
        Found,
        NotFound,
        Error
    };
    struct OneResult
    {
        One status = One::NotFound;
        Json::Value body;  // объект оценки при Found
    };
    using OneCb = std::function<void(OneResult)>;

    static void gradeById(drogon::orm::DbClientPtr db, int64_t gradeId,
                          OneCb cb);

    // GET /v1/exams → {"exams": [...]} — session_plan пользователя
    // (02 §5.2 «Экран „Сессия“: консультации/экзамены/зачёты/пересдачи»).
    // kind задан CHECK-ограничением 02 §4.1 — все четыре значения
    // (consultation/exam/credit/retake) принадлежат экрану, фильтра нет;
    // порядок по starts_at (idx_session_plan_user (user_id, starts_at) —
    // 02 §4.2). Плана сессии нет (Аня, staff) — 200 с пустым exams.
    static void exams(drogon::orm::DbClientPtr db, int64_t userId, ResultCb cb);
};

}  // namespace para
