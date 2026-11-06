#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include <drogon/orm/DbClient.h>

namespace para
{

// Автомат статусов заявки (Э5 шаг 1; гайд BE §7 «Э5 — заявки и справки»;
// 05 §3 Э5 п.1). Матрица допустимых переходов — constexpr-константа
// src/models/Applications.h (05 §2: матрица переходов статусов живёт в
// models/); сервис принимает решение о переходе и применяет его.
//
// Применение перехода = одна атомарная операция (drogon Transaction):
//   UPDATE applications SET status = to WHERE id AND status = from
//   INSERT INTO application_events (from_status, to_status, comment,
//                                   author_id)   — таймлайн заявки (С2.4,
//                                                 С7.7);
// UPDATE условный по текущему статусу: если строка не совпала, статус
// изменили параллельно — InvalidTransition (409), ничего не изменено.
// updated_at обновляется той же строкой UPDATE (очередь сотрудника
// сортируется по idx_applications_queue (status, updated_at) — 02 §4.2).
//
// Владение заявкой apply() НЕ проверяет: контроллер с path-id обязан ДО
// вызова применить OwnershipGuard::check(Table::Applications) — правило
// код-ревью в шапке OwnershipGuard.h (guard различает 404/403). Выборка
// статуса здесь — по PK без user_id, как gradeById (Э4): не дублировать
// проверку владения SQL-условием мимо guard'а.
//
// Исходный NULL→submitted-событие подачи (fixtures.sql) пишет контроллер при
// создании/отправке заявки (Э5 шаг 2) — apply() работает только со
// статусными переходами существующей заявки; from_status события здесь
// всегда ненулевой.
class AppWorkflow
{
  public:
    // Допустим ли переход по матрице (делегирует constexpr-предикату
    // src/models/Applications.h). Не ходит в БД — контроллер может
    // использовать для ранней валидации 409.
    static bool canTransition(std::string_view from, std::string_view to);

    enum class Outcome
    {
        Applied,            // статус изменён, событие таймлайна записано
        InvalidTransition,  // переход вне матрицы ИЛИ статус изменили
                            // параллельно → контроллер отвечает 409
        NotFound,           // заявки с таким id нет → 404
        Error,              // ошибка БД → 500, fail-closed
    };

    struct ApplyResult
    {
        Outcome outcome = Outcome::Error;
        std::string fromStatus;  // статус заявки на момент отказа — для
                                 // сообщения 409 («из X в Y нельзя»); при
                                 // NotFound — пустая строка
        int64_t eventId = 0;     // id строки таймлайна при Applied (SELECT
                                 // last_insert_rowid() внутри транзакции) —
                                 // caller отдаёт событие в теле ответа без
                                 // «последнего события» на гонках; 0 иначе
    };
    using ApplyCb = std::function<void(ApplyResult)>;

    // Хук Э5 шаг 3 — выполняется В ТОЙ ЖЕ транзакции после записи события
    // (до коммита): автор уведомления/прочие атомарные последствия перехода
    // (05 §3 Э5 п.3: «в одной транзакции: UPDATE + событие + INSERT в
    // notifications»). Аргументы: tx (захватывается в колбэки хука — держит
    // транзакцию живой до их завершения) и fail() — ошибка БД в хуке:
    // откат ВСЕЙ транзакции (статус + событие + последствия) и исход Error.
    // nullptr — без последствий (обычный переход).
    using AfterEventTx = std::function<void(
        const std::shared_ptr<drogon::orm::Transaction>& tx,
        const std::function<void()>& fail)>;

    // Применить переход: чтение текущего статуса → проверка матрицы →
    // транзакция (условный UPDATE + событие таймлайна [+ afterEvent]).
    // comment — причина отклонения / вопрос деканата (05 §3 Э5:
    // «rejected (с причиной)»); nullopt → SQL NULL (колонка nullable по DDL
    // 02 §4.1). authorId — пользователь сессии, инициировавший переход
    // (студент при отмене/ответе, сотрудник при смене статуса). Колбэк
    // стреляет ровно один раз.
    static void apply(drogon::orm::DbClientPtr db,
                      int64_t applicationId,
                      const std::string& toStatus,
                      const std::optional<std::string>& comment,
                      int64_t authorId,
                      ApplyCb cb,
                      AfterEventTx afterEvent = nullptr);
};

}  // namespace para
