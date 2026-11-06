#pragma once

#include <functional>

#include <drogon/orm/DbClient.h>

namespace para {

// Фоновые напоминания Э6 (гайд BE §9; 02 §4 поток 4; таблицы миграции
// 0003_workflow: deadlines.reminded, deadline_reminders.remind_at/sent_at):
//  (а) базовое напоминание «за 1 день»: дедлайны с due_at в ближайшие 24 ч
//      ((now; now+24h]) и reminded = 0 → уведомление type='deadline'
//      участникам — групповой дедлайн всем студентам группы
//      (profiles.group_id; staff без профиля не попадает), персональный —
//      владельцу (user_id; он же приоритетен, если заполнены оба поля) —
//      затем reminded = 1 («базовая рассылка по дедлайну сделана», DDL);
//  (б) персональные «Напомнить за день» (кнопка дедлайн-радара, 5.2):
//      deadline_reminders с наступившим remind_at (<= now) и
//      sent_at IS NULL → уведомление владельцу → sent_at;
//  (в) утренний дайджест тихих часов — NotifyDispatcher::flushDigest.
// Каждая рассылка идёт через NotifyDispatcher (шаг 1): тумблеры и тихие
// часы применяются к КАЖДОМУ получателю — ночное напоминание копится в
// утренний дайджест (02 §4 поток 4), delivery уже не зависит от тика.
// Идемпотентность тика (шаг 2 Э6): условные UPDATE меток
// (reminded = 1 / sent_at) после факта рассылки — повторный тик и
// перезапуск приложения не создают дублей.
class ReminderService
{
  public:
    struct TickStats
    {
        int deadlineNotices = 0;  // (а) дедлайнов, по которым сделана рассылка
        int personalReminders = 0;  // (б) персональных напоминаний
        std::size_t digestDelivered = 0;  // (в) отправлено из очереди дайджеста
        std::size_t digestHeld = 0;       // (в) вернулись в очередь (ещё ночь)
    };
    using TickCb = std::function<void(TickStats)>;

    // Один тик таймера (main.cpp: registerBeginningAdvice →
    // getLoop()->runEvery(60 с)): (а) → (б) → (в) линейно. cb стреляет
    // ровно один раз; ошибка БД фазы не прерывает остальные (лог, фаза
    // даёт 0 — недоставленное повторит следующий тик по меткам).
    static void runMinuteTick(drogon::orm::DbClientPtr db, TickCb cb);

    // Фазы отдельно — юнит-тесты вызывают синхронно (promise/future);
    // runMinuteTick — их конвейер. cb — число обработанных строк.
    static void basicDeadlineRound(
        drogon::orm::DbClientPtr db,
        std::function<void(int deadlinesHandled)> cb);
    static void personalReminderRound(drogon::orm::DbClientPtr db,
                                      std::function<void(int sent)> cb);
};

}  // namespace para
