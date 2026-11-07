#include "services/NotificationsFeedService.h"

#include <drogon/drogon.h>

#include "services/NotifyDispatcher.h"  // notifyTypeFromString/Name — словарь CHECK 0004

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// $N-плейсхолдеры: порядок появления в тексте SQL = порядок биндинга
// (урок Э5 шаг 2: sqlite3-клиент drogon биндит позиционно).

// Строка ленты → JSON (02 §4.1: body/deep_link/read_at — TEXT NULL →
// JSON null, не пустая строка).
Json::Value notificationJson(const Row& r)
{
    Json::Value n(Json::objectValue);
    n["id"] = r["id"].as<int64_t>();
    n["type"] = r["type"].as<std::string>();
    n["title"] = r["title"].as<std::string>();
    if (!r["body"].isNull())
        n["body"] = r["body"].as<std::string>();
    else
        n["body"] = Json::Value();
    if (!r["deep_link"].isNull())
        n["deep_link"] = r["deep_link"].as<std::string>();
    else
        n["deep_link"] = Json::Value();
    if (!r["read_at"].isNull())
        n["read_at"] = r["read_at"].as<std::string>();
    else
        n["read_at"] = Json::Value();  // непрочитанное
    n["created_at"] = r["created_at"].as<std::string>();
    return n;
}

constexpr const char* kSelectBase =
    "SELECT id, type, title, body, deep_link, read_at, created_at "
    "FROM notifications ";

// Страница (≤ kPageSize строк) → тело 200 (лента + next_cursor — id
// последней строки, только если есть 21-я — детектор следующей страницы).
// Общий для обеих веток (с курсором и без) — структура ответа не расходится.
Json::Value pageBodyFrom(const Result& rows)
{
    Json::Value arr(Json::arrayValue);
    std::optional<int64_t> next;
    const std::size_t cnt = rows.size();
    for (std::size_t i = 0; i < cnt && i < NotificationsFeedService::kPageSize;
         ++i)
        arr.append(notificationJson(rows[i]));
    if (cnt > NotificationsFeedService::kPageSize)
        next = rows[NotificationsFeedService::kPageSize - 1]["id"]
                   .as<int64_t>();
    Json::Value body(Json::objectValue);
    body["notifications"] = std::move(arr);
    body["next_cursor"] =
        next.has_value() ? Json::Value(*next) : Json::Value();
    return body;
}

}  // namespace

void NotificationsFeedService::feed(DbClientPtr db,
                                    int64_t userId,
                                    const Query& q,
                                    FeedCb cb)
{
    if (!db || userId <= 0)
    {
        cb(FeedResult{});  // Error — fail-closed
        return;
    }

    // Словарь CHECK 0004 — тот же, что пишет NotifyDispatcher
    // (notifyTypeFromString): тип вне словаря → BadRequest, а не пустая
    // выдача (стиль ?status=bogus → 400 ApplicationsCtrl).
    std::optional<NotifyType> typeEnum;
    if (q.type.has_value())
    {
        typeEnum = notifyTypeFromString(*q.type);
        if (!typeEnum.has_value())
        {
            FeedResult r;
            r.status = FeedStatus::BadRequest;
            r.error = "type — ожидается schedule, deadline, application "
                      "или announcement";
            cb(std::move(r));
            return;
        }
    }

    // Курсор — id строки ЭТОГО пользователя: перечитываем её created_at
    // (составная пагинация (created_at, id) — шапка класса). Чужой/несущ.
    // id → BadRequest (не раскрываем содержимое чужих строк).
    std::optional<std::string> cursorCreated;
    if (q.cursorId.has_value())
    {
        *db << "SELECT created_at FROM notifications WHERE id = $1 AND "
               "user_id = $2"
            << *q.cursorId << userId >>
            [db, userId, q, typeEnum, cb](const Result& rows) mutable {
                if (rows.empty())
                {
                    FeedResult r;
                    r.status = FeedStatus::BadRequest;
                    r.error = "cursor — уведомление не найдено";
                    cb(std::move(r));
                    return;
                }
                const std::string created =
                    rows[0]["created_at"].as<std::string>();

                // WHERE собирается по частям; нумерация $N — по порядку
                // появления в тексте (нумератор n).
                std::string sql = kSelectBase;
                sql += "WHERE user_id = $1";
                int n = 2;
                if (q.unreadOnly)
                    sql += " AND read_at IS NULL";
                if (typeEnum.has_value())
                    sql += " AND type = $" + std::to_string(n++);
                if (q.cursorId.has_value())
                {
                    sql += " AND (created_at < $" + std::to_string(n++) +
                           " OR (created_at = $" + std::to_string(n++) +
                           " AND id < $" + std::to_string(n++) + "))";
                }
                sql += " ORDER BY created_at DESC, id DESC LIMIT $" +
                       std::to_string(n++);

                auto binder = *db << std::move(sql);
                binder << userId;  // $1
                if (typeEnum.has_value())
                    binder << notifyTypeName(*typeEnum);
                if (q.cursorId.has_value())
                {
                    binder << created;      // created_at < $c
                    binder << created;      // created_at = $c
                    binder << *q.cursorId;  // id < $cid
                }
                binder << static_cast<int>(kPageSize + 1);
                binder >>
                    [cb](const Result& rows) {
                        // cb копируется в ОБЕ ветки (урок Э2 шаг 1: move в
                        // success-ветке оставил бы error-ветке пустой
                        // std::function) — как в RoomsService::cancelById.
                        cb(FeedResult{FeedStatus::Ok, pageBodyFrom(rows), {}});
                    } >>
                    [cb](const DrogonDbException& e) {
                        LOG_ERROR << "NotificationsFeedService: feed: "
                                  << e.base().what();
                        cb(FeedResult{});  // Error → 500 (fail-closed)
                    };
            } >>
            [cb](const DrogonDbException& e) {
                LOG_ERROR << "NotificationsFeedService: cursor: "
                          << e.base().what();
                cb(FeedResult{});  // Error → 500 (fail-closed)
            };
        return;
    }

    // Без курсора — та же сборка WHERE (общая структура, отдельная ветка
    // без перечитывания курсора).
    std::string sql = kSelectBase;
    sql += "WHERE user_id = $1";
    int n = 2;
    if (q.unreadOnly)
        sql += " AND read_at IS NULL";
    if (typeEnum.has_value())
        sql += " AND type = $" + std::to_string(n++);
    sql += " ORDER BY created_at DESC, id DESC LIMIT $" + std::to_string(n++);

    auto binder = *db << std::move(sql);
    binder << userId;
    if (typeEnum.has_value())
        binder << notifyTypeName(*typeEnum);
    binder << static_cast<int>(kPageSize + 1);
    binder >>
        [cb](const Result& rows) {
            cb(FeedResult{FeedStatus::Ok, pageBodyFrom(rows), {}});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NotificationsFeedService: feed: " << e.base().what();
            cb(FeedResult{});  // Error → 500 (fail-closed)
        };
}

void NotificationsFeedService::markRead(DbClientPtr db,
                                        int64_t userId,
                                        int64_t notificationId,
                                        std::function<void(ReadStatus)> cb)
{
    if (!db || userId <= 0 || notificationId <= 0)
    {
        cb(ReadStatus::Error);
        return;
    }
    // Условный UPDATE (идемпотентность): непрочитанное → помечено (Ok),
    // уже прочитанное → 0 строк (AlreadyRead → тот же 204 контроллером).
    // user_id в WHERE — скоуп в самом запросе (02 §6.3 п.2); read_at —
    // серверное strftime, как DEFAULT DDL (строка клиента не принимается).
    // cb копируется в ОБЕ ветки (урок Э2 шаг 1: move в success-ветке
    // оставил бы error-ветке пустой std::function).
    *db << "UPDATE notifications SET read_at = "
           "strftime('%Y-%m-%dT%H:%M:%SZ','now') "
           "WHERE id = $1 AND user_id = $2 AND read_at IS NULL"
        << notificationId << userId >>
        [cb](const Result& result) {
            cb(result.affectedRows() > 0 ? ReadStatus::Ok
                                         : ReadStatus::AlreadyRead);
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NotificationsFeedService: markRead: "
                      << e.base().what();
            cb(ReadStatus::Error);  // fail-closed → 500
        };
}

void NotificationsFeedService::markAllRead(
    DbClientPtr db, int64_t userId, std::function<void(MarkAllResult)> cb)
{
    if (!db || userId <= 0)
    {
        cb(MarkAllResult{});
        return;
    }
    *db << "UPDATE notifications SET read_at = "
           "strftime('%Y-%m-%dT%H:%M:%SZ','now') "
           "WHERE user_id = $1 AND read_at IS NULL"
        << userId >>
        [cb](const Result& result) {
            cb(MarkAllResult{true,
                             static_cast<long long>(result.affectedRows())});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NotificationsFeedService: markAllRead: "
                      << e.base().what();
            cb(MarkAllResult{});  // fail-closed → 500
        };
}

}  // namespace para
