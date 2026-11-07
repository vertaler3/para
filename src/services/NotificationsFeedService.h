#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// ============================================================================
// Лента уведомлений — закрытие пробела Э6 (02 §5.2):
//
//   GET  /v1/notifications            — лента текущего пользователя
//       (query: unread=1, type, cursor)                      → 200;
//   POST /v1/notifications/{id}/read  — отметить прочитанным (guard
//       контроллера различает 403/404)                       → 204 · 403 · 404;
//   POST /v1/notifications/read-all   — пометить всё прочитанным → 204.
//
// SQL — только здесь, параметрами ($N; AGENTS §5.5, 02 §6.4). Формат
// ошибок — 02 §5.1, его собирает контроллер.
//
// Лента ПЛОСКАЯ: ORDER BY created_at DESC, id DESC (02 §5.2 «группировка
// по типам» — опция ОТОБРАЖЕНИЯ экрана 03 §4.6, задача FE-дорожки; API
// отдаёт единый список — зафиксировано в журнале сессии).
//
// Пагинация курсором — СОСТАВНАЯ (id, created_at): created_at имеет
// гранулярность 1 с (strftime DDL), у одного владельца возможны две строки
// в одной секунде; простой порог «id < cursor» при сортировке по
// created_at пропускал бы/дублировал строки, когда порядок id противоречит
// порядку времени. Условие страницы:
//   created_at < $c OR (created_at = $c AND id < $cid),
// где $c/$cid — created_at/id строки-курсора (сервис перечитывает их по id
// СКОПИРОВАВ по user_id — чужой id курсора не раскрывает содержимое,
// → BadRequest). Клиент передаёт next_cursor из ответа обратно в ?cursor=.
//
// markRead/markAllRead — идемпотентны: уже прочитанное (read_at IS NOT
// NULL) не трогается и отвечает тем же 204 (повторный тап/повторная
// синхронизация FE безопасны). user_id остаётся в WHERE самих UPDATE —
// скоуп в самом запросе (02 §6.3 п.2: guard — третья ступень, не замена
// WHERE user_id). read_at — серверное время strftime (единый формат DDL),
// не строка клиента.
// ============================================================================
class NotificationsFeedService
{
  public:
    // Параметры ленты (query 02 §5.2). type — сырое значение ?type=;
    // словарь CHECK миграции 0004 ('schedule','deadline','application',
    // 'announcement') проверяет сервис (notifyTypeFromString — единый
    // словарь с NotifyDispatcher) → BadRequest.
    struct Query
    {
        bool unreadOnly = false;          // ?unread=1 — только read_at IS NULL
        std::optional<std::string> type;  // ?type= — фильтр по словарю CHECK
        std::optional<int64_t> cursorId;  // ?cursor= — id последнего загруж.
    };

    enum class FeedStatus
    {
        Ok,          // 200 — body готов
        BadRequest,  // 400 — недопустимый type / несуществующий курсор
        Error,       // 500 — ошибка БД (fail-closed)
    };
    struct FeedResult
    {
        FeedStatus status = FeedStatus::Error;
        Json::Value body;       // Ok: {"notifications":[…], "next_cursor":…}
        std::string error;      // BadRequest: человекочитаемая причина
    };
    using FeedCb = std::function<void(FeedResult)>;

    // Страница ленты (константа приложения, как kPageSize Announcements):
    // LIMIT pageSize+1 — строка-детектор следующей страницы.
    static constexpr std::size_t kPageSize = 20;

    static void feed(drogon::orm::DbClientPtr db,
                     int64_t userId,
                     const Query& q,
                     FeedCb cb);

    // markRead(id, userId): UPDATE read_at=strftime('now') WHERE id И
    // user_id И read_at IS NULL (условный — идемпотентность). Ok — строка
    // помечена; AlreadyRead — условие не совпало (уже прочитано; после
    // Own guard'а контроллера это единственная причина) → тот же 204;
    // Error — ошибка БД → 500.
    enum class ReadStatus
    {
        Ok,
        AlreadyRead,
        Error,
    };
    static void markRead(drogon::orm::DbClientPtr db,
                         int64_t userId,
                         int64_t notificationId,
                         std::function<void(ReadStatus)> cb);

    struct MarkAllResult
    {
        bool ok = false;
        long long marked = 0;  // сколько строк переведено в прочитанные
    };

    // markAllRead(userId): read_at=strftime('now') WHERE user_id И
    // read_at IS NULL; marked=0 — всё уже прочитано (тот же 204).
    static void markAllRead(drogon::orm::DbClientPtr db,
                            int64_t userId,
                            std::function<void(MarkAllResult)> cb);
};

}  // namespace para
