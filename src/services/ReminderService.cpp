#include "services/ReminderService.h"

#include <atomic>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <drogon/drogon.h>

#include "services/NotifyDispatcher.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Окно базового напоминания «за 1 день»: due_at в (now; now+24h], reminded=0
// (02 §4 поток 4: «дедлайны с due_at в окне и reminded = 0»). Даты — ISO
// «YYYY-MM-DDTHH:MM:SSZ» (02 §4.1, формат strftime DDL) — сравнение строк
// лексикографично. Параметры now/now+24h биндятся из C++ (тот же тестовый
// шов времени NotifyDispatcher::nowUtc, что у решения о тихих часах).
// due_at в прошлом не выбирается: напоминание «за 1 день» о прошедшем сроке
// не имеет смысла, а reminded останется 0 до конца окна... не выбирается
// вовсе — окно открыто только в будущее (зафиксировано юнит-тестом).
const std::string kDueWindowSql =
    "SELECT id, group_id, user_id, title, due_at FROM deadlines "
    "WHERE reminded = 0 AND due_at > $1 AND due_at <= $2";

// Групповой дедлайн — всем студентам группы (02 §4 поток 4: «всем участникам
// группы»). Профили есть только у студентов (02 §4.1: profiles — карточка
// студента с обязательной группой), staff без профиля не попадает.
const std::string kGroupRecipientsSql =
    "SELECT user_id FROM profiles WHERE group_id = $1";

// Метка «базовая рассылка по дедлайну сделана» (DDL комментарий reminded).
// Условный UPDATE по (id, reminded=0) — идемпотентность: параллельный/повторный
// тик ничего не изменит.
const std::string kMarkRemindedSql =
    "UPDATE deadlines SET reminded = 1 WHERE id = $1 AND reminded = 0";

// Персональные «Напомнить за день» (5.2): remind_at наступил, ещё не послано.
const std::string kPendingRemindersSql =
    "SELECT r.user_id, r.deadline_id, d.title, d.due_at "
    "FROM deadline_reminders r JOIN deadlines d ON d.id = r.deadline_id "
    "WHERE r.remind_at <= $1 AND r.sent_at IS NULL";

// Метка sent_at — условный UPDATE (sent_at IS NULL): повторный тик после
// рассылки не перепишет и не создаст дубля.
const std::string kMarkSentSql =
    "UPDATE deadline_reminders SET sent_at = $1 "
    "WHERE user_id = $2 AND deadline_id = $3 AND sent_at IS NULL";

// deep_link — экран дедлайн-радара (5.2); тап → объект (02 §4.1).
const char* kDeadlineDeepLink = "/deadlines";

void fire(const std::function<void()>& cb)
{
    if (cb)
        cb();
}

// Пометить дедлайн рассланным и закрыть его обработку.
void markRemindedAndDone(DbClientPtr db, int64_t deadlineId,
                         std::function<void()> done)
{
    *db << kMarkRemindedSql << deadlineId >>
        [done](const Result&) { fire(done); } >>
        [done, deadlineId](const DrogonDbException& e) {
            LOG_ERROR << "ReminderService: mark reminded(" << deadlineId
                      << "): " << e.base().what();
            fire(done);  // метка не поставлена — следующий тик повторит
        };
}

// Отправить уведомление о дедлайне всем получателям; когда исходы всех
// notify() известны — reminded = 1. Исход каждого получателя решает
// диспетчер (тумблер/тихие часы/дайджест) — тик ждёт только завершения.
void dispatchToRecipients(DbClientPtr db,
                          const std::vector<int64_t>& recipients,
                          int64_t deadlineId,
                          const std::string& title,
                          const std::string& body,
                          std::function<void()> done)
{
    if (recipients.empty())
    {
        // Некому напоминать (пустая группа) — дедлайн всё равно рассланный.
        markRemindedAndDone(db, deadlineId, std::move(done));
        return;
    }
    auto remaining = std::make_shared<std::atomic<int>>(
        static_cast<int>(recipients.size()));
    for (const int64_t userId : recipients)
    {
        NotifyInput in;
        in.userId = userId;
        in.type = NotifyType::Deadline;
        in.title = title;
        in.body = body;
        in.deepLink = kDeadlineDeepLink;
        NotifyDispatcher::notify(db, in,
                                 [db, deadlineId, remaining, done](
                                     NotifyOutcome) {
                                     // Исход (Sent/Deferred/Suppressed) —
                                     // рассылка по дедлайну состоялась.
                                     if (remaining->fetch_sub(1) == 1)
                                         markRemindedAndDone(db, deadlineId,
                                                             std::move(done));
                                 });
    }
}

// Получатели одного дедлайна: персональный (user_id заполнен) — владелец;
// групповой — студенты группы.
void dispatchDeadline(DbClientPtr db, const Row& row,
                      std::function<void()> done)
{
    const int64_t id = row["id"].as<int64_t>();
    const std::string title = row["title"].as<std::string>();
    const std::string dueAt = row["due_at"].as<std::string>();
    const std::string body = "Срок: " + dueAt;  // базовое «за 1 день»

    if (!row["user_id"].isNull())
    {
        dispatchToRecipients(db, {row["user_id"].as<int64_t>()}, id, title,
                             body, std::move(done));
        return;
    }
    if (row["group_id"].isNull())
    {
        LOG_WARN << "ReminderService: дедлайн " << id
                 << " без group_id и user_id — получателей нет";
        markRemindedAndDone(db, id, std::move(done));
        return;
    }
    const int64_t groupId = row["group_id"].as<int64_t>();
    *db << kGroupRecipientsSql << groupId >>
        [db, id, title, body, done](const Result& rows) mutable {
            std::vector<int64_t> recipients;
            recipients.reserve(rows.size());
            for (const auto& r : rows)
                recipients.push_back(r["user_id"].as<int64_t>());
            dispatchToRecipients(db, recipients, id, title, body,
                                 std::move(done));
        } >>
        [done, groupId](const DrogonDbException& e) {
            LOG_ERROR << "ReminderService: recipients(" << groupId
                      << "): " << e.base().what();
            fire(done);  // без метки: следующий тик повторит рассылку
        };
}

}  // namespace

void ReminderService::basicDeadlineRound(DbClientPtr db,
                                         std::function<void(int)> cb)
{
    const auto now = NotifyDispatcher::nowUtc();
    const std::string nowIso = NotifyDispatcher::toIsoUtc(now);
    const std::string horizonIso =
        NotifyDispatcher::toIsoUtc(now + std::chrono::hours(24));
    *db << kDueWindowSql << nowIso << horizonIso >>
        [db, cb = std::move(cb)](const Result& rows) {
            if (rows.empty())
            {
                cb(0);
                return;
            }
            const int total = static_cast<int>(rows.size());
            auto remaining = std::make_shared<std::atomic<int>>(total);
            for (const auto& row : rows)
            {
                dispatchDeadline(db, row,
                                 [remaining, total, cb] {
                                     if (remaining->fetch_sub(1) == 1)
                                         cb(total);
                                 });
            }
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ReminderService: due window: " << e.base().what();
            cb(0);  // фаза не удалась — следующий тик повторит по reminded=0
        };
}

void ReminderService::personalReminderRound(DbClientPtr db,
                                            std::function<void(int)> cb)
{
    const std::string nowIso = NotifyDispatcher::nowIsoUtc();
    *db << kPendingRemindersSql << nowIso >>
        [db, cb = std::move(cb)](const Result& rows) {
            if (rows.empty())
            {
                cb(0);
                return;
            }
            const int total = static_cast<int>(rows.size());
            auto remaining = std::make_shared<std::atomic<int>>(total);
            for (const auto& row : rows)
            {
                const int64_t userId = row["user_id"].as<int64_t>();
                const int64_t deadlineId = row["deadline_id"].as<int64_t>();
                const std::string title = row["title"].as<std::string>();
                const std::string dueAt = row["due_at"].as<std::string>();
                NotifyInput in;
                in.userId = userId;
                in.type = NotifyType::Deadline;
                in.title = title;
                in.body = "Ваше напоминание — срок: " + dueAt;
                in.deepLink = kDeadlineDeepLink;
                NotifyDispatcher::notify(
                    db, in,
                    [db, userId, deadlineId, remaining, total,
                     cb](NotifyOutcome) {
                        // Метка sent_at — после факта рассылки (исход решён
                        // диспетчером); cb копируется в обе ветки (урок Э2).
                        *db << kMarkSentSql << NotifyDispatcher::nowIsoUtc()
                            << userId << deadlineId >>
                            [remaining, total, cb](const Result&) {
                                if (remaining->fetch_sub(1) == 1)
                                    cb(total);
                            } >>
                            [remaining, total, cb,
                             userId, deadlineId](const DrogonDbException& e) {
                                LOG_ERROR << "ReminderService: mark sent("
                                          << userId << "," << deadlineId
                                          << "): " << e.base().what();
                                if (remaining->fetch_sub(1) == 1)
                                    cb(total);
                            };
                    });
            }
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ReminderService: pending reminders: "
                      << e.base().what();
            cb(0);
        };
}

void ReminderService::runMinuteTick(DbClientPtr db, TickCb cb)
{
    auto stats = std::make_shared<TickStats>();
    basicDeadlineRound(db, [db, stats, cb](int deadlines) {
        stats->deadlineNotices = deadlines;
        personalReminderRound(db, [db, stats, cb](int sent) {
            stats->personalReminders = sent;
            NotifyDispatcher::flushDigest(
                db, [stats, cb](NotifyDispatcher::DigestFlush flush) {
                    stats->digestDelivered = flush.delivered;
                    stats->digestHeld = flush.held;
                    if (cb)
                        cb(*stats);
                });
        });
    });
}

}  // namespace para
