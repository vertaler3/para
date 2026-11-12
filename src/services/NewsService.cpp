#include "services/NewsService.h"

#include <cctype>
#include <cstdio>
#include <ctime>
#include <utility>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// $N-плейсхолдеры: порядок появления в тексте SQL = порядок биндинга
// (урок Э5 шаг 2: sqlite3-клиент drogon биндит позиционно).

// Словарь CHECK 0007 — тот же для ленты (?category=) и публикации
// (kind='news'): official | conference | org | event.
const char* const kCategories[] = {"official", "conference", "org", "event"};

// Строка news → JSON (02 §4.1: TEXT NULL → JSON null, не пустая строка).
Json::Value newsJson(const Row& r)
{
    Json::Value n(Json::objectValue);
    n["id"] = r["id"].as<int64_t>();
    n["category"] = r["category"].as<std::string>();
    n["title"] = r["title"].as<std::string>();
    if (!r["body"].isNull())
        n["body"] = r["body"].as<std::string>();
    else
        n["body"] = Json::Value();
    if (!r["event_at"].isNull())
        n["event_at"] = r["event_at"].as<std::string>();
    else
        n["event_at"] = Json::Value();  // новость без события
    if (!r["location"].isNull())
        n["location"] = r["location"].as<std::string>();
    else
        n["location"] = Json::Value();
    n["audience"] = r["audience"].as<std::string>();
    if (!r["audience_value"].isNull())
        n["audience_value"] = r["audience_value"].as<std::string>();
    else
        n["audience_value"] = Json::Value();  // NULL при audience='all'
    n["published_at"] = r["published_at"].as<std::string>();
    return n;
}

// Страница (≤ kPageSize строк) → тело 200 (лента + next_cursor — id
// последней строки, только если есть 21-я — детектор следующей страницы).
// Общий для обеих веток (с курсором и без) — структура ответа не
// расходится (приём NotificationsFeedService::pageBodyFrom).
Json::Value pageBodyFrom(const Result& rows)
{
    Json::Value arr(Json::arrayValue);
    std::optional<int64_t> next;
    const std::size_t cnt = rows.size();
    for (std::size_t i = 0; i < cnt && i < NewsService::kPageSize; ++i)
        arr.append(newsJson(rows[i]));
    if (cnt > NewsService::kPageSize)
        next = rows[NewsService::kPageSize - 1]["id"].as<int64_t>();
    Json::Value body(Json::objectValue);
    body["news"] = std::move(arr);
    body["next_cursor"] =
        next.has_value() ? Json::Value(*next) : Json::Value();
    return body;
}

// Строгий «YYYY-MM-DDTHH:MM:SSZ» — формат DDL/фикстур/
// SessionService::utcNow (приём RoomsService::isIsoZ): все сравнения меток
// этой ленты лексикографические по тексту, вход валидируется строго, чтобы
// сравнение не соврало (2026-11-20T10:00 vs 2026-11-20 10:00).
bool isIsoZ(const std::string& s)
{
    if (s.size() != 20 || s[4] != '-' || s[7] != '-' || s[10] != 'T' ||
        s[13] != ':' || s[16] != ':' || s[19] != 'Z')
        return false;
    const auto digits = [&s](std::size_t i) {
        return std::isdigit(static_cast<unsigned char>(s[i])) != 0;
    };
    for (std::size_t i = 0; i < 4; ++i)
        if (!digits(i))
            return false;
    for (const std::size_t i : {5u, 8u, 11u, 14u, 17u})
        if (!digits(i) || !digits(i + 1))
            return false;
    const int hour = std::stoi(s.substr(11, 2));
    const int minute = std::stoi(s.substr(14, 2));
    const int second = std::stoi(s.substr(17, 2));
    if (hour > 23 || minute > 59 || second > 59)
        return false;
    // Календарность даты (приём RoomsService::validDate): timegm
    // нормализует 2026-02-30 → 02.03 — обратная отрисовка обязана совпасть
    // байт-в-байт, иначе даты не было.
    int y = 0, mo = 0, d = 0;
    if (std::sscanf(s.substr(0, 10).c_str(), "%d-%d-%d", &y, &mo, &d) != 3 ||
        y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31)
        return false;
    std::tm tm{};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_isdst = 0;
    const std::time_t t = timegm(&tm);
    if (t < 0)
        return false;
    std::tm back{};
    gmtime_r(&t, &back);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &back);
    return s.compare(0, 10, buf) == 0;
}

// Фрагмент WHERE — видимость аудитории (шапка NewsService.h): студент с
// профилем видит 'all' + свою группу + свой курс; профиля нет (staff,
// Р-6) — все (прецедент AnnouncementsService::list). audience_value —
// TEXT (DDL 0007), биндится СТРОКОЙ (сравнение TEXT = TEXT; '1' ≠ 1 в
// SQLite). group_id/course NULL или 0 → '0' не совпадает с реальными
// id > 0 — «нет группы → групповые новости не видны». n — нумератор $N
// вызова (инкрементируется на число занятых плейсхолдеров).
std::string visibilitySql(bool hasProfile, int& n)
{
    if (!hasProfile)
        return "1=1";
    const std::string p1 = "$" + std::to_string(n);
    const std::string p2 = "$" + std::to_string(n + 1);
    n += 2;
    return "(audience = 'all' OR (audience = 'group' AND audience_value = " +
           p1 + ") OR (audience = 'course' AND audience_value = " + p2 + "))";
}

// Основная страница ленты: вызывается после профиля (и, при курсоре, —
// перечитывания строки-курсора). cursor — (published_at, id) строки-курсора
// или nullopt (первая страница). cb копируется в ОБЕ ветки (урок Э2/Э5а:
// move в success-ветке оставил бы error-ветке пустой std::function).
void runPage(DbClientPtr db,
             bool hasProfile,
             int64_t groupId,
             int64_t course,
             const std::optional<std::string>& category,
             const std::optional<std::pair<std::string, int64_t>>& cursor,
             NewsService::FeedCb cb)
{
    std::string sql =
        "SELECT id, category, title, body, event_at, location, audience, "
        "audience_value, published_at FROM news WHERE ";
    int n = 1;
    sql += visibilitySql(hasProfile, n);
    if (category.has_value())
        sql += " AND category = $" + std::to_string(n++);
    if (cursor.has_value())
    {
        sql += " AND (published_at < $" + std::to_string(n++) +
               " OR (published_at = $" + std::to_string(n++) +
               " AND id < $" + std::to_string(n++) + "))";
    }
    sql += " ORDER BY published_at DESC, id DESC LIMIT $" + std::to_string(n++);

    auto binder = *db << std::move(sql);
    if (hasProfile)
    {
        binder << std::to_string(groupId);  // audience_value — TEXT
        binder << std::to_string(course);
    }
    if (category.has_value())
        binder << *category;
    if (cursor.has_value())
    {
        binder << cursor->first;   // published_at < $c
        binder << cursor->first;   // published_at = $c
        binder << cursor->second;  // id < $cid
    }
    binder << static_cast<int>(NewsService::kPageSize + 1);
    binder >>
        [cb](const Result& rows) {
            cb(NewsService::FeedResult{NewsService::FeedStatus::Ok,
                                       pageBodyFrom(rows), {}});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NewsService: feed: " << e.base().what();
            cb(NewsService::FeedResult{});  // Error → 500 (fail-closed)
        };
}

}  // namespace

bool NewsService::isValidCategory(const std::string& category)
{
    for (const char* const c : kCategories)
        if (category == c)
            return true;
    return false;
}

void NewsService::feed(DbClientPtr db,
                       const SessionInfo& session,
                       const Query& q,
                       FeedCb cb)
{
    if (!db || session.userId <= 0)
    {
        cb(FeedResult{});  // Error — fail-closed
        return;
    }

    // Словарь CHECK 0007: категория вне словаря → BadRequest, а не пустая
    // выдача (стиль ?type у NotificationsFeedService, ?status у заявок).
    if (q.category.has_value() && !isValidCategory(*q.category))
    {
        cb(FeedResult{FeedStatus::BadRequest, Json::Value(),
                      "category — ожидается official, conference, org или "
                      "event"});
        return;
    }

    // Профиль сессии: группа/курс для фильтра аудитории — тот же SELECT,
    // что AnnouncementsService::list (staff — профиля нет, Р-6 → видит
    // все). cb копируется во все ветки обеих цепочек (стреляет ровно
    // одна).
    *db << "SELECT group_id, course FROM profiles WHERE user_id = $1"
        << session.userId >>
        [db, q, cb](const Result& rows) mutable {
            const bool has = !rows.empty();
            int64_t groupId = 0, course = 0;
            if (has)
            {
                if (!rows[0]["group_id"].isNull())
                    groupId = rows[0]["group_id"].as<int64_t>();
                if (!rows[0]["course"].isNull())
                    course = rows[0]["course"].as<int64_t>();
            }

            if (!q.cursorId.has_value())
            {
                runPage(db, has, groupId, course, q.category, std::nullopt,
                        std::move(cb));
                return;
            }

            // Курсор — id строки СВОЕЙ видимой ленты: у news нет владельца
            // (публичная сущность), «своя» = видимая аудитории сессии —
            // перечитываем published_at по id С УСЛОВИЕМ ВИДИМОСТИ; чужой
            // по аудитории/несуществующий → BadRequest (не раскрываем
            // содержимое невидимых строк; паттерн перечитывания —
            // NotificationsFeedService, там владелец user_id).
            int n = 2;
            std::string sql =
                "SELECT published_at FROM news WHERE id = $1 AND " +
                visibilitySql(has, n);
            auto binder = *db << std::move(sql);
            binder << *q.cursorId;
            if (has)
            {
                binder << std::to_string(groupId);
                binder << std::to_string(course);
            }
            binder >>
                [db, has, groupId, course, q, cb](const Result& rows) mutable {
                    if (rows.empty())
                    {
                        cb(FeedResult{FeedStatus::BadRequest, Json::Value(),
                                      "cursor — новость не найдена"});
                        return;
                    }
                    runPage(db, has, groupId, course, q.category,
                            std::make_pair(
                                rows[0]["published_at"].as<std::string>(),
                                *q.cursorId),
                            std::move(cb));
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "NewsService: cursor: " << e.base().what();
                    cb(FeedResult{});  // Error → 500 (fail-closed)
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NewsService: profile: " << e.base().what();
            cb(FeedResult{});  // fail-closed: ошибка БД → 500, не «видит все»
        };
}

void NewsService::publish(DbClientPtr db, const Draft& d, PublishCb cb)
{
    if (!db)
    {
        cb(PublishResult{});
        return;
    }

    // Синхронная валидация значений (до похода в БД): непустые title/body
    // уже гарантированы контроллером (общая ветка обоих kind), здесь —
    // поля, специфичные для новости. Стиль RoomsService::create —
    // InvalidBody синхронно, без асинхронного шума.
    if (!isValidCategory(d.category))
    {
        cb(PublishResult{PublishStatus::BadRequest, Json::Value(),
                         "category — ожидается official, conference, org или "
                         "event"});
        return;
    }
    if (d.eventAt.has_value() && !isIsoZ(*d.eventAt))
    {
        cb(PublishResult{
            PublishStatus::BadRequest, Json::Value(),
            "event_at — ожидается YYYY-MM-DDTHH:MM:SSZ"});
        return;
    }
    if (d.location.has_value() && d.location->empty())
    {
        cb(PublishResult{PublishStatus::BadRequest, Json::Value(),
                         "location — ожидается непустая строка"});
        return;
    }
    if (!d.audienceAll &&
        ((d.audienceKind != "group" && d.audienceKind != "course") ||
         d.audienceValue <= 0))
    {
        cb(PublishResult{PublishStatus::BadRequest, Json::Value(),
                         "audience — ожидается \"all\" или ровно одна цель "
                         "[\"group:<id>\"] / [\"course:<id>\"]"});
        return;
    }

    const std::string audience = d.audienceAll ? std::string("all")
                                               : d.audienceKind;
    const std::optional<std::string> audienceValue =
        d.audienceAll
            ? std::nullopt
            : std::optional<std::string>(std::to_string(d.audienceValue));

    // published_at — серверное «сейчас»: strftime UTC (формат DDL 0004
    // read_at; DEFAULT у 0007 сознательно нет — момент задаёт публикация).
    // INSERT — autocommit; id — перечитыванием по естественному ключу
    // (урок Э5а: не last_insert_rowid — пул соединений). Push и уведомлений
    // НЕТ: news — редакционная лента (01 §4.5), таймер/дайджест Э6 её не
    // касаются.
    *db << "INSERT INTO news (category, title, body, event_at, location, "
           "audience, audience_value, published_at) "
           "VALUES ($1, $2, $3, $4, $5, $6, $7, "
           "strftime('%Y-%m-%dT%H:%M:%SZ','now'))"
        << d.category << d.title << d.body << d.eventAt << d.location
        << audience << audienceValue >>
        [db, d, cb](const Result&) mutable {
            *db << "SELECT id, category, title, body, event_at, location, "
                   "audience, audience_value, published_at FROM news "
                   "WHERE category = $1 AND title = $2 AND body = $3 "
                   "ORDER BY id DESC LIMIT 1"
                << d.category << d.title << d.body >>
                [cb](const Result& rows) mutable {
                    if (rows.empty())
                    {
                        cb(PublishResult{});  // Error — fail-closed
                        return;
                    }
                    cb(PublishResult{PublishStatus::Ok, newsJson(rows[0]),
                                     {}});
                } >>
                [cb](const DrogonDbException& e) {
                    LOG_ERROR << "NewsService: re-select: " << e.base().what();
                    cb(PublishResult{});  // fail-closed → 500
                };
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "NewsService: insert: " << e.base().what();
            cb(PublishResult{});  // fail-closed → 500
        };
}

}  // namespace para
