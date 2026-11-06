#include "services/AppWorkflow.h"

#include <memory>
#include <string>

#include <drogon/drogon.h>

#include "models/Applications.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Текущий статус заявки по PK — БЕЗ user_id-условия: владение уже проверил
// OwnershipGuard в контроллере (см. заголовок класса; паттерн gradeById Э4).
const std::string kCurrentStatusSql =
    "SELECT status FROM applications WHERE id = $1";

// Условный UPDATE по (id, текущий статус): совпадение статуса в WHERE —
// защита от гонки параллельных смен (0 строк → InvalidTransition, ничего
// не изменено). updated_at — тем же форматом, что DDL (strftime, 02 §4.1),
// для очереди сотрудника idx_applications_queue (status, updated_at).
const std::string kUpdateSql =
    "UPDATE applications SET status = $1, "
    "updated_at = strftime('%Y-%m-%dT%H:%M:%SZ','now') "
    "WHERE id = $2 AND status = $3";

// Событие таймлайна (02 §4.1): from/to/comment/author; `at` — DEFAULT DDL.
// comment — std::optional<std::string> (SqlBinder v1.9.14 биндит
// std::optional: nullopt → SQL NULL; проверено компиляцией в образе).
const std::string kInsertEventSql =
    "INSERT INTO application_events "
    "(application_id, from_status, to_status, comment, author_id) "
    "VALUES ($1, $2, $3, $4, $5)";

// id последней вставленной строки на соединении — читается ВНУТРИ
// транзакции (соединение закреплено за ней, межзапросной гонки нет).
const std::string kLastRowIdSql = "SELECT last_insert_rowid() AS id";

}  // namespace

bool AppWorkflow::canTransition(std::string_view from, std::string_view to)
{
    return models::canApplicationTransition(from, to);
}

void AppWorkflow::apply(drogon::orm::DbClientPtr db,
                        int64_t applicationId,
                        const std::string& toStatus,
                        const std::optional<std::string>& comment,
                        int64_t authorId,
                        ApplyCb cb,
                        AfterEventTx afterEvent)
{
    // 1. Текущий статус. cb копируется в обе ветки цепочки (стреляет ровно
    //    одна); move в первую лямбду оставил бы второй пустой std::function →
    //    bad_function_call на error-пути (урок Э2 шаг 1, OwnershipGuard.cpp).
    *db << kCurrentStatusSql << applicationId >>
        [db, applicationId, toStatus, comment, authorId, cb,
         afterEvent](const Result& rows) {
            if (rows.empty())
            {
                // Несуществующая заявка → 404 (владение не раскрываем).
                cb(ApplyResult{Outcome::NotFound, {}});
                return;
            }
            const std::string from = rows[0]["status"].as<std::string>();

            // 2. Решение автомата — матрица из src/models/Applications.h.
            if (!canTransition(from, toStatus))
            {
                cb(ApplyResult{Outcome::InvalidTransition, from});
                return;
            }

            // 3. Транзакция: условный UPDATE + событие таймлайна атомарно
            //    (05 §3 Э5: смена статуса — «в одной транзакции: UPDATE +
            //    событие»; уведомление автору — шаг 2 Э5, не здесь).
            //    once — единственный выстрел колбэка: commit-callback
            //    стреляет при уничтожении tx и не должен дублировать
            //    error-ветки; atomic под shared_ptr (atomic некопируем).
            //    eventId — id записанного события (шаг 2 Э5: тело 201 у
            //    messages строится по id, не по «последнему событию»).
            auto once = std::make_shared<std::atomic_bool>(false);
            auto eventId = std::make_shared<int64_t>(0);
            auto fire = [cb, applicationId, once,
                         eventId](Outcome outcome, const std::string& from) {
                bool expected = false;
                if (!once->compare_exchange_strong(expected, true))
                    return;
                if (outcome == Outcome::Error)
                {
                    LOG_WARN << "AppWorkflow::apply(" << applicationId
                             << "): ошибка БД, транзакция откатывается";
                }
                cb(ApplyResult{outcome, from, *eventId});
            };

            db->newTransactionAsync([fire, from, toStatus, comment, authorId,
                                     applicationId, eventId, afterEvent](
                                        const std::shared_ptr<
                                            Transaction>& transaction) {
                if (!transaction)
                {
                    // Таймаут ожидания соединения (02 §5.1: 500 без деталей).
                    LOG_ERROR << "AppWorkflow::apply(" << applicationId
                              << "): транзакция недоступна (timeout)";
                    fire(Outcome::Error, from);
                    return;
                }
                // fail хука: откат всей транзакции + исход Error (fire
                // защищён once — не задублирует commit-callback).
                const std::function<void()> fail = [fire, from,
                                                    transaction]() {
                    fire(Outcome::Error, from);
                    transaction->rollback();
                };

                auto updateOk = [tx = transaction, from, toStatus, comment,
                                 authorId, applicationId, eventId, afterEvent,
                                 fire, fail](const Result& r) {
                    if (r.affectedRows() == 0)
                    {
                        // Гонка: статус изменили параллельно — ничего не
                        // изменено, явный откат и 409.
                        LOG_WARN << "AppWorkflow::apply(" << applicationId
                                 << "): статус '" << from
                                 << "' изменён параллельно";
                        fire(Outcome::InvalidTransition, from);
                        tx->rollback();
                        return;
                    }

                    // 4. Событие таймлайна (application_events, 02 §4.1) +
                    // его id (last_insert_rowid() на том же соединении
                    // транзакции — до коммита). tx захвачен в колбэки
                    // INSERT и SELECT: последний держатель уничтожается
                    // после завершения SELECT → деструктор транзакции
                    // коммитит (TransactionImpl v1.9.14: commit — в
                    // деструкторе), commit-callback отдаёт Applied.
                    *tx << kInsertEventSql << applicationId << from
                        << toStatus << comment << authorId >>
                        [tx, eventId, afterEvent,
                         fail](const Result&) {
                            *tx << kLastRowIdSql >>
                                [tx, eventId, afterEvent,
                                 fail](const Result& rows) {
                                    if (!rows.empty())
                                        *eventId =
                                            rows[0]["id"].as<int64_t>();
                                    // 5. Атомарные последствия перехода
                                    //    (уведомление автору — Э5 шаг 3)
                                    //    в той же транзакции, до коммита.
                                    if (afterEvent)
                                        afterEvent(tx, fail);
                                } >>
                                [tx](const DrogonDbException& e) {
                                    // Событие записано, id не прочитан —
                                    // не фейлим Applied: eventId остаётся 0.
                                    LOG_WARN << "AppWorkflow::apply: "
                                             << "select event id: "
                                             << e.base().what();
                                };
                        } >>
                        [tx, from, fire](const DrogonDbException& e) {
                            LOG_ERROR << "AppWorkflow::apply: insert event: "
                                      << e.base().what();
                            fire(Outcome::Error, from);
                            tx->rollback();
                        };
                };

                *transaction << kUpdateSql << toStatus << applicationId
                             << from >>
                    std::move(updateOk) >>
                    [tx = transaction, from, fire,
                     applicationId](const DrogonDbException& e) {
                        LOG_ERROR << "AppWorkflow::apply(" << applicationId
                                  << "): update status: " << e.base().what();
                        fire(Outcome::Error, from);
                        tx->rollback();
                    };

                // Commit-callback: success → Applied; commit-fail → Error
                // (после rollback callback не стреляет вовсе — v1.9.14).
                transaction->setCommitCallback(
                    [fire, from](bool committed) {
                        fire(committed ? Outcome::Applied : Outcome::Error,
                             from);
                    });
            });
        } >>
        [cb, applicationId](const DrogonDbException& e) {
            LOG_ERROR << "AppWorkflow::apply(" << applicationId
                      << "): select status: " << e.base().what();
            cb(ApplyResult{Outcome::Error, {}});  // fail-closed → 500
        };
}

}  // namespace para
