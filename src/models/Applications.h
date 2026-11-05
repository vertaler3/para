#pragma once

#include <array>
#include <string_view>

namespace para::models
{

// ============================================================================
// Константы автомата заявок (Э5). Дерево 05 §2: models/ — «constexpr-
// константы: каталог application_types, матрица переходов статусов, бейдж
// концепта»; этот файл несёт матрицу переходов статусов и словарь статусов
// (каталог application_types и бейдж — отдельные шаги Э5).
//
// Словарь статусов — дословно CHECK DDL applications (02 §4.1,
// миграция 0003_workflow): draft, submitted, accepted, in_progress,
// need_info, ready, issued, rejected, cancelled.
//
// Матрица допустимых переходов — списком пар «из → в» (05 §3 Э5 п.1:
// «задавать переходы списком пар, не цепочкой»), значения 05 §3 Э5 дословно:
//
//   draft       → submitted (отправка) | cancelled (отмена студентом)
//   submitted   → accepted | rejected (с причиной) | cancelled
//   accepted    → in_progress | rejected | cancelled
//   in_progress → ready | need_info (вопрос деканата) | rejected | cancelled
//   need_info   → in_progress (ответ студента — С7.6 01: «статус
//                 возвращается „В работе"») | rejected | cancelled
//   ready       → issued (единственный переход из ready)
//
//   issued / rejected / cancelled — терминальные: переходов ИЗ них нет
//   (05 §3 Э5; 02 §5.2 поток 2: «ready → issued да; issued → in_progress
//   нет → 409»).
//
// Отклонение (rejected) и отмена студентом (cancelled) включены — они
// предусмотрены 01 §4.4 («ветки „отклонена", „отменена студентом"») и
// словарем CHECK DDL 02 §4.1; rejected оформляется с причиной в комментарии
// события (05 §3 Э5: «rejected (с причиной)»).
//
// Недопустимый переход — не ошибка БД: сервис AppWorkflow возвращает
// InvalidTransition, контроллер отвечает 409 в формате 02 §5.1.
// ============================================================================

// Словарь статусов — 9 значений CHECK DDL 02 §4.1 (единый источник для
// контроллеров и тестов; строка статуса в БД ограничена тем же CHECK).
inline constexpr std::array<std::string_view, 9> kApplicationStatuses{
    "draft", "submitted", "accepted", "in_progress", "need_info",
    "ready", "issued", "rejected", "cancelled"};

// Пара «из → в» матрицы переходов.
struct StatusTransition
{
    std::string_view from;
    std::string_view to;
};

// Матрица допустимых переходов (16 пар, порядок — по 05 §3 Э5 п.1).
inline constexpr std::array<StatusTransition, 16>
    kApplicationStatusTransitions{{
        // draft → submitted (отправка) | cancelled (отмена студентом)
        {"draft", "submitted"},
        {"draft", "cancelled"},
        // submitted → accepted | rejected (с причиной) | cancelled
        {"submitted", "accepted"},
        {"submitted", "rejected"},
        {"submitted", "cancelled"},
        // accepted → in_progress | rejected | cancelled
        {"accepted", "in_progress"},
        {"accepted", "rejected"},
        {"accepted", "cancelled"},
        // in_progress → ready | need_info (вопрос деканата) | rejected |
        // cancelled
        {"in_progress", "ready"},
        {"in_progress", "need_info"},
        {"in_progress", "rejected"},
        {"in_progress", "cancelled"},
        // need_info → in_progress (ответ студента — С7.6) | rejected |
        // cancelled
        {"need_info", "in_progress"},
        {"need_info", "rejected"},
        {"need_info", "cancelled"},
        // ready → issued (единственный переход из ready)
        {"ready", "issued"},
    }};

// Допустим ли переход по матрице: линейный проход по 16 парам — constexpr,
// без состояний. Используется сервисом AppWorkflow (решение о переходе),
// контроллерами (ранняя валидация 409 до обращения к БД) и юнит-тестами
// (исчерпывающий перебор всех пар словаря статусов).
constexpr bool canApplicationTransition(std::string_view from,
                                        std::string_view to)
{
    for (const auto& t : kApplicationStatusTransitions)
        if (t.from == from && t.to == to)
            return true;
    return false;
}

}  // namespace para::models
