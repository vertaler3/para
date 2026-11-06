#pragma once

#include <chrono>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/SettingsService.h"

namespace para {

// Тип уведомления — словарь CHECK миграции 0004 (02 §4.1):
// type IN ('schedule','deadline','application','announcement').
enum class NotifyType
{
    Schedule,
    Deadline,
    Application,
    Announcement,
};

// Имя типа для БД/WS-payload ('schedule'… — словарь CHECK). НЕ «toString»:
// свободный toString(NotifyType) в namespace para перехватывается ADL'ом
// внутри doctest::detail::stringifyBinaryExpr и ломает компиляцию CHECK'ов
// над enum'ами (поймано на сборке тестов Э6).
const char* notifyTypeName(NotifyType type);

// Обратное преобразование для HTTP-входа шага 4 Э6 (переносы от staff);
// вне словаря CHECK → nullopt (недопустимый тип не дойдёт до БД).
std::optional<NotifyType> notifyTypeFromString(const std::string& s);

// Исход решения NotifyDispatcher::notify (02 §4.5):
//   - Sent — строка записана в notifications (источник истины) и пушнута
//     живому WS-соединению, если оно есть (колбэк шага 3 установлен);
//   - DeferredToDigest — тихие часы, не срочное: уведомление НЕ создаётся
//     сразу — попадает в очередь утреннего дайджеста (01 §5.6; механизм
//     дайджеста — шаг 2 Э6, сейчас очередь/пометка);
//   - SuppressedByToggle — тумблер типа выключен (settings, 02 §4.1);
//   - Error — неверные аргументы или ошибка БД; ничего не записано.
enum class NotifyOutcome
{
    Sent,
    DeferredToDigest,
    SuppressedByToggle,
    Error,
};

// Элемент очереди утреннего дайджеста (разбирается шагом 2 Э6).
struct DeferredNotification
{
    int64_t userId = 0;
    NotifyType type = NotifyType::Schedule;
    std::string title;
    std::string body;
    std::string deepLink;
    std::string deferredAtUtc;  // «YYYY-MM-DDTHH:MM:SSZ» — момент откладки
};

// Вход notify(). deep_link заполняется ВСЕГДА (тап → карточка переноса /
// таймлайн заявки — 02 §4.1: комментарий колонки); пустой deepLink — Error.
// eventStartsAtUtc — starts_at события переноса «YYYY-MM-DDTHH:MM:SSZ»:
// только для type=schedule, из него считается срочность «ближайшей пары»
// (01 §5.6). Все времена — UTC (как весь контур, 02 §4.3).
struct NotifyInput
{
    int64_t userId = 0;
    NotifyType type = NotifyType::Schedule;
    std::string title;
    std::string body;  // пустая строка → SQL NULL (DDL: body TEXT)
    std::string deepLink;
    std::optional<std::string> eventStartsAtUtc;
};

// NotifyDispatcher — единая точка создания уведомлений (02 §4.5: «пишет
// уведомление в БД — источник истины — и пушит живым соединениям»; гайд
// BE §9). Правило тихих часов живёт здесь (02 §5.6), а не у вызывающих.
//
// Границы шага 1 Э6:
//  - реестр WS-соединений появится в шаге 3 (WsCtrl) — сейчас точка
//    интеграции setPushFn: без установленного колбэка push не стреляет;
//  - отложенные тихими часами уведомления копятся во внутренней очереди
//    (takePendingDigest) — утренний разбор подключит шаг 2;
//  - транзакционные вставки Э5 (смена статуса заявки, блокировка брони)
//    остаются прямыми INSERT в своей транзакции — гейт Э5 требует
//    атомарности «переход + уведомление»; их перенос в диспетчер —
//    отдельное решение, здесь не делалось.
class NotifyDispatcher
{
  public:
    // Точка интеграции шага 3 (WsCtrl): колбэк пушит payload живому
    // WS-соединению пользователя, если оно есть (наличие соединения
    // решает реестр шага 3, не диспетчер). Для announcement не зовётся —
    // объявления push-ем не отправляются (01 §4.5, решение по каналам).
    using PushFn =
        std::function<void(int64_t userId, const Json::Value& payload)>;
    static void setPushFn(PushFn fn);

    // Часы — тестовый шов (фиксированное «сейчас» в проверках тихих часов
    // и срочности); в приложении не устанавливается → system_clock::now.
    using ClockFn = std::function<std::chrono::system_clock::time_point()>;
    static void setClock(ClockFn fn);
    static std::chrono::system_clock::time_point nowUtc();

    // Тихие часы (02 §5.6; 01 §5.6): minuteOfDayUtc ∈ [0;1440) попадает в
    // интервал [from,to) из quiet_hours_json (дефолт DDL
    // {"from":"23:00","to":"07:00"}); допускаются синонимы start/end.
    // Интервал может переходить через полночь (23:00–07:00); from == to —
    // пустой интервал. Невалидный JSON/ключи → интервал пуст (не
    // задерживаем: валидность значения гарантирует applyPatch настроек).
    static bool inQuietHours(const std::string& quietHoursJson,
                             int minuteOfDayUtc);

    // Срочность (01 §5.6): срочный перенос — событие начинается в течение
    // 24 часов (0 ≤ starts−now ≤ 24ч, граница включена). Нечитаемая дата,
    // дата в прошлом или отсутствие starts_at → не срочное (тихие часы
    // задерживают — безопасный исход).
    static bool isUrgentSchedule(
        const std::optional<std::string>& eventStartsAtUtc,
        const std::chrono::system_clock::time_point& now);

    // Чистая функция решения (тестируется напрямую): 1) тумблер типа —
    // выключен → SuppressedByToggle; 2) announcement — всегда Sent (нет
    // тумблера, мимо тихих часов и push — 01 §4.5/§5.6); 3) тихие часы и
    // не срочное → DeferredToDigest; иначе Sent.
    static NotifyOutcome decide(const SettingsRecord& settings,
                                NotifyType type,
                                bool urgent,
                                int minuteOfDayUtc);

    // Единая точка создания уведомления: settings пользователя → решение →
    // (Sent) INSERT в notifications и WS-push; (DeferredToDigest) — в
    // очередь дайджеста, БД не трогается; (SuppressedByToggle) — ничего;
    // (Error) — ничего (fail-closed: ошибка БД не оставляет строку без
    // push и не пушит без строки). cb стреляет ровно один раз.
    static void notify(drogon::orm::DbClientPtr db,
                       const NotifyInput& in,
                       std::function<void(NotifyOutcome)> cb);

    // Очередь дайджеста: отложенные тихими часами уведомления. Шаг 2 Э6
    // заберёт takePendingDigest() утром и создаст записи/дайджест; сейчас
    // очередь — единственное их место (в notifications они не создаются).
    static std::vector<DeferredNotification> takePendingDigest();
    static std::size_t pendingDigestCount();

    // Тестовый шов: сброс push-колбэка, часов и очереди (между кейсами —
    // статики живут в одном бинарнике на все TEST_CASE).
    static void resetForTests();

  private:
    // INSERT успешен: WS-push живому соединению, если реестр дал колбэк
    // (шаг 3); announcement мимо push. Вызывается только после записи в БД.
    static void pushLive(const NotifyInput& in);
};

}  // namespace para
