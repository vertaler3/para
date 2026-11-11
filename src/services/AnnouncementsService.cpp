#include "services/AnnouncementsService.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

constexpr std::size_t kPageSize = 20;

// $N-плейсхолдеры: порядок появления в тексте = порядок биндинга
// (урок Э5 шаг 2; sqlite3-клиент drogon биндит позиционно).

Json::Value announcementJson(const Row& r)
{
    Json::Value a(Json::objectValue);
    a["id"] = r["id"].as<int64_t>();
    a["author_id"] = r["author_id"].as<int64_t>();
    if (!r["author_fio"].isNull())
        a["author"] = r["author_fio"].as<std::string>();
    a["title"] = r["title"].as<std::string>();
    a["body"] = r["body"].as<std::string>();
    // audience — TEXT 'all' | JSON-массив; в ответе — вложенное JSON-значение
    // (валидность гарантирована parseAudience при публикации и DEFAULT 'all').
    Json::Value audience;
    if (!Json::Reader().parse(r["audience"].as<std::string>(), audience))
        audience = "all";
    a["audience"] = audience;
    a["published_at"] = r["published_at"].as<std::string>();
    return a;
}

// Реквизиты автора (ФИО) — LEFT JOIN profiles (staff — без профиля).
const std::string kSelectBase =
    "SELECT a.id, a.author_id, a.title, a.body, a.audience, a.published_at, "
    "       p.fio AS author_fio "
    "FROM announcements a "
    "LEFT JOIN profiles p ON p.user_id = a.author_id ";

}  // namespace

bool AnnouncementsService::parseAudience(const std::string& audienceJson,
                                         Audience& out)
{
    out = Audience{};
    if (audienceJson == "all")
        return true;  // дефолт DDL 0003
    Json::Value parsed;
    if (!Json::Reader().parse(audienceJson, parsed) || !parsed.isArray())
        return false;
    out.all = false;
    for (const auto& item : parsed)
    {
        if (!item.isString())
            return false;
        const std::string s = item.asString();
        const std::size_t colon = s.find(':');
        if (colon == std::string::npos || colon == 0)
            return false;
        const std::string kind = s.substr(0, colon);
        if (kind != "group" && kind != "course")
            return false;
        try
        {
            const int64_t id = std::stoll(s.substr(colon + 1));
            if (id <= 0)
                return false;
            out.targets.emplace_back(kind, id);
        }
        catch (const std::exception&)
        {
            return false;  // не число
        }
    }
    return !out.targets.empty();
}

std::string AnnouncementsService::serializeAudience(const Audience& a)
{
    if (a.all)
        return "all";
    Json::Value arr(Json::arrayValue);
    for (const auto& [kind, id] : a.targets)
        arr.append(kind + ":" + std::to_string(id));
    Json::StreamWriterBuilder b;
    b["indentation"] = "";
    return Json::writeString(b, arr);
}

void AnnouncementsService::list(DbClientPtr db,
                                const SessionInfo& session,
                                std::optional<int64_t> cursor,
                                ResultCb cb)
{
    if (!db)
    {
        cb(Result{});
        return;
    }

    // Профиль сессии: группа/курс (staff — профиля нет, Р-6 → видит все).
    *db << "SELECT group_id, course FROM profiles WHERE user_id = $1"
        << session.userId >>
        // cb КОПИРУЕТСЯ во все ветки обеих цепочек (стреляет ровно одна);
        // move в success-ветке оставил бы error-ветке пустой
        // std::function → bad_function_call (урок Э2, Э8 шаг 5).
        [db, cursor, cb](
            const drogon::orm::Result& rows) mutable {
            bool has = false;
            int64_t groupId = 0, course = 0;
            if (!rows.empty())
            {
                has = true;
                if (!rows[0]["group_id"].isNull())
                    groupId = rows[0]["group_id"].as<int64_t>();
                if (!rows[0]["course"].isNull())
                    course = rows[0]["course"].as<int64_t>();
            }

            // WHERE: студента — 'all' или массив содержит его цель (instr по
            // '"group:<id>"' — кавычки исключают ложные совпадения префиксов
            // вида group:1x); staff — без условий аудитории. Нумерация $N —
            // по порядку появления в тексте (урок Э5).
            std::string sql = kSelectBase + "WHERE ";
            int n = 1;
            if (has)
            {
                const std::string p1 = "$" + std::to_string(n);
                const std::string p2 = "$" + std::to_string(n + 1);
                n += 2;
                sql += "(a.audience = 'all' OR instr(a.audience, " + p1 +
                       ") > 0 OR instr(a.audience, " + p2 + ") > 0)";
            }
            else
            {
                sql += "1=1";
            }
            if (cursor.has_value())
            {
                sql += " AND a.id < $" + std::to_string(n);
                ++n;
            }
            sql += " ORDER BY a.id DESC LIMIT $" + std::to_string(n);

            auto binder = *db << std::move(sql);
            if (has)
            {
                binder << "\"group:" + std::to_string(groupId) + "\"";
                binder << "\"course:" + std::to_string(course) + "\"";
            }
            if (cursor.has_value())
                binder << *cursor;
            binder << static_cast<int>(kPageSize + 1);
            binder >>
                [cb](const drogon::orm::Result& rows) mutable {
                    Json::Value arr(Json::arrayValue);
                    std::optional<int64_t> next;
                    const std::size_t cnt = rows.size();
                    for (std::size_t i = 0; i < cnt && i < kPageSize; ++i)
                        arr.append(announcementJson(rows[i]));
                    if (cnt > kPageSize)
                        next = rows[kPageSize - 1]["id"].as<int64_t>();
                    Json::Value body(Json::objectValue);
                    body["announcements"] = std::move(arr);
                    body["next_cursor"] = next.has_value()
                                              ? Json::Value(*next)
                                              : Json::Value();
                    cb(Result{true, std::move(body)});
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "AnnouncementsService: list: "
                              << e.base().what();
                    cb(Result{});
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "AnnouncementsService: profile: " << e.base().what();
            cb(Result{});  // fail-closed: ошибка БД → 500, не «видит все»
        };
}

void AnnouncementsService::publish(DbClientPtr db,
                                   int64_t staffId,
                                   const std::string& title,
                                   const std::string& body,
                                   const Audience& audience,
                                   ResultCb cb)
{
    if (!db)
    {
        cb(Result{});
        return;
    }
    const std::string audienceText = serializeAudience(audience);
    // published_at — DEFAULT DDL (strftime UTC); push/уведомлений НЕТ
    // (01 §4.5: объявления живут в ленте и дайджесте). INSERT — autocommit:
    // после него строка закоммичена и видна с любого соединения пула;
    // id — перечитывание по естественному ключу (урок Э5а: не
    // last_insert_rowid — пул соединений).
    *db << "INSERT INTO announcements (author_id, title, body, audience) "
           "VALUES ($1, $2, $3, $4)"
        << staffId << title << body << audienceText >>
        [db, staffId, title, body, cb](
            const drogon::orm::Result&) mutable {
            *db << kSelectBase +
                       "WHERE a.author_id = $1 AND a.title = $2 AND "
                       "a.body = $3 ORDER BY a.id DESC LIMIT 1"
                << staffId << title << body >>
                [cb](const drogon::orm::Result& rows) mutable {
                    if (rows.empty())
                    {
                        cb(Result{});
                        return;
                    }
                    cb(Result{true, announcementJson(rows[0])});
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "AnnouncementsService: re-select: "
                              << e.base().what();
                    cb(Result{});
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "AnnouncementsService: insert: " << e.base().what();
            cb(Result{});
        };
}

}  // namespace para
