#include "services/AssistantService.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>

#include <drogon/HttpClient.h>
#include <drogon/drogon.h>
#include <drogon/orm/Exception.h>

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "services/DashboardService.h"
#include "services/GradesService.h"
#include "services/NewsService.h"
#include "services/ScheduleService.h"

using namespace drogon;
using drogon::orm::DrogonDbException;

namespace para {

namespace {

std::string envOr(const char* name)
{
    const char* v = std::getenv(name);
    return v == nullptr ? std::string() : std::string(v);
}

// Время «сейчас» в формате проекта (ISO-8601 UTC «…Z»): сравнение строк
// лексикографическое — как в DashboardService/ReminderService.
std::string nowIso()
{
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tm);
    return buf;
}

// «…Z» → time_t (UTC); -1 — не разобрали.
std::time_t parseIso(const std::string& iso)
{
    std::tm tm{};
    if (iso.size() < 16 ||
        std::sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d", &tm.tm_year,
                    &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min) != 5)
        return -1;
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    return timegm(&tm);
}

constexpr std::time_t kMskOffset = 3 * 3600;  // UTC+3 круглый год

std::string fmtMsk(const std::string& iso, const char* pattern)
{
    const std::time_t t = parseIso(iso);
    if (t < 0)
        return iso;
    const std::time_t local = t + kMskOffset;
    std::tm tm{};
    gmtime_r(&local, &tm);
    char buf[32];
    std::strftime(buf, sizeof buf, pattern, &tm);
    return buf;
}

std::string mskDay(const std::string& iso)
{
    return fmtMsk(iso, "%Y-%m-%d");
}

const char* weekdayRu(const std::string& date)
{
    static const char* kNames[] = {"воскресенье", "понедельник", "вторник",
                                   "среда",       "четверг",     "пятница",
                                   "суббота"};
    const std::time_t t = parseIso(date + "T12:00");
    if (t < 0)
        return "";
    std::tm tm{};
    gmtime_r(&t, &tm);
    return kNames[tm.tm_wday];
}

std::string lessonKind(const std::string& k)
{
    if (k == "lecture")
        return "лекция";
    if (k == "seminar")
        return "семинар";
    return k;
}

std::string planKind(const std::string& k)
{
    if (k == "consultation")
        return "консультация";
    if (k == "exam")
        return "экзамен";
    if (k == "credit")
        return "зачёт";
    if (k == "retake")
        return "пересдача";
    if (k == "work")
        return "работа";
    return k;
}

std::string str(const Json::Value& v, const char* key)
{
    const auto& x = v[key];
    return x.isString() ? x.asString() : std::string();
}

void renderLesson(std::string& out, const Json::Value& e)
{
    out += "- " + AssistantService::mskTime(str(e, "starts_at")) + "–" +
           AssistantService::mskTime(str(e, "ends_at")) + " " +
           str(e, "discipline") + " (" + lessonKind(str(e, "kind")) +
           "), ауд. " + str(e, "room");
    if (!str(e, "building").empty())
        out += ", " + str(e, "building");
    const std::string status = str(e, "status");
    if (status == "cancelled")
        out += " — ОТМЕНЕНА";
    else if (status == "moved")
    {
        out += " — ПЕРЕНЕСЕНА";
        std::string was;
        if (!str(e, "prev_starts_at").empty())
            was += AssistantService::mskDateTime(str(e, "prev_starts_at"));
        if (!str(e, "prev_room").empty())
            was += (was.empty() ? "ауд. " : ", ауд. ") + str(e, "prev_room");
        if (!was.empty())
            out += " (было: " + was + ")";
    }
    else if (status == "substituted")
        out += " — замена преподавателя";
    out += "\n";
}

}  // namespace

// --- Настройки ---------------------------------------------------------------

std::string AssistantService::unquoteEnv(const std::string& raw)
{
    if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'')
        return raw.substr(1, raw.size() - 2);
    if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"')
        return raw;
    std::string out;
    out.reserve(raw.size());
    for (std::size_t i = 1; i + 1 < raw.size(); ++i)
    {
        const char c = raw[i];
        if (c == '\\' && i + 2 < raw.size())
        {
            const char n = raw[++i];
            switch (n)
            {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                default: out += '\\'; out += n; break;
            }
            continue;
        }
        out += c;
    }
    return out;
}

std::string AssistantService::resolveModel(const std::string& model,
                                           const std::string& folderId)
{
    if (model.empty() || model.find("://") != std::string::npos ||
        folderId.empty())
        return model;
    // «yandexgpt-lite» → gpt://<каталог>/yandexgpt-lite/latest;
    // «yandexgpt-lite/rc» (версия указана) — без добавки /latest.
    const bool hasVersion = model.find('/') != std::string::npos;
    return "gpt://" + folderId + "/" + model + (hasVersion ? "" : "/latest");
}

AssistantService::Config AssistantService::configFromEnv()
{
    Config c;
    c.url = unquoteEnv(envOr("ASSISTANT_API_URL"));
    c.key = unquoteEnv(envOr("ASSISTANT_API_KEY"));
    c.model = resolveModel(unquoteEnv(envOr("ASSISTANT_MODEL")),
                           unquoteEnv(envOr("ASSISTANT_FOLDER_ID")));
    c.systemPrompt = unquoteEnv(envOr("ASSISTANT_SYSTEM_PROMPT"));
    return c;
}

// --- Валидация -----------------------------------------------------------------

std::size_t AssistantService::utf8Length(const std::string& s)
{
    std::size_t n = 0;
    for (const unsigned char c : s)
        if ((c & 0xC0) != 0x80)  // не байт-продолжение
            ++n;
    return n;
}

AssistantService::Input AssistantService::validate(const Json::Value* body,
                                                   std::string& out)
{
    out.clear();
    if (body == nullptr || !body->isObject() || !(*body)["message"].isString())
        return Input::Empty;
    const std::string raw = (*body)["message"].asString();
    const auto first = raw.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return Input::Empty;
    const auto last = raw.find_last_not_of(" \t\r\n");
    out = raw.substr(first, last - first + 1);
    if (utf8Length(out) > kMaxMessageChars)
    {
        out.clear();
        return Input::TooLong;
    }
    return Input::Ok;
}

// --- Лимитер -----------------------------------------------------------------

int64_t AssistantService::RateLimiter::acquire(int64_t userId, int64_t nowSec)
{
    std::lock_guard<std::mutex> lock(mu_);
    auto& q = hits_[userId];
    while (!q.empty() && q.front() <= nowSec - kWindowSec)
        q.pop_front();
    if (static_cast<int>(q.size()) >= kMax)
        return std::max<int64_t>(1, q.front() + kWindowSec - nowSec);
    q.push_back(nowSec);
    return 0;
}

AssistantService::RateLimiter& AssistantService::RateLimiter::instance()
{
    static RateLimiter limiter;
    return limiter;
}

// --- Контекст ------------------------------------------------------------------

std::string AssistantService::mskTime(const std::string& iso)
{
    return fmtMsk(iso, "%H:%M");
}

std::string AssistantService::mskDateTime(const std::string& iso)
{
    return fmtMsk(iso, "%d.%m %H:%M");
}

std::string AssistantService::renderContext(const ContextData& d)
{
    const std::string tomorrow = ScheduleService::addDays(d.today, 1);
    std::string out;
    out += "ДАННЫЕ ПОРТАЛА ДЛЯ ЭТОГО СТУДЕНТА (вымышленные; время московское):\n";
    out += "Сегодня " + d.today + ", " + weekdayRu(d.today) + ".\n";

    for (const auto& [day, label] :
         {std::pair{d.today, "Расписание на сегодня"},
          std::pair{tomorrow, "Расписание на завтра"}})
    {
        out += std::string("\n") + label + " (" + day + ", " + weekdayRu(day) +
               "):\n";
        bool any = false;
        if (d.events.isArray())
            for (const auto& e : d.events)
                if (mskDay(str(e, "starts_at")) == day)
                {
                    renderLesson(out, e);
                    any = true;
                }
        if (!any)
            out += "- пар нет\n";
    }

    out += "\nБлижайшие консультации, экзамены и зачёты:\n";
    bool anyExam = false;
    const std::string now = nowIso();
    if (d.exams.isArray())
        for (const auto& e : d.exams)
            if (str(e, "starts_at") >= now)
            {
                out += "- " + mskDateTime(str(e, "starts_at")) + " " +
                       planKind(str(e, "kind")) + ": " + str(e, "discipline") +
                       ", ауд. " + str(e, "room") + "\n";
                anyExam = true;
            }
    if (!anyExam)
        out += "- нет\n";

    out += "\nАктивные дедлайны:\n";
    if (d.deadlines.isArray() && !d.deadlines.empty())
        for (const auto& x : d.deadlines)
            out += "- до " + mskDateTime(str(x, "due_at")) + ": " +
                   str(x, "title") + " (" + planKind(str(x, "kind")) + ")\n";
    else
        out += "- нет\n";

    out += "\nЗаявки в работе:\n";
    if (d.applications.isArray() && !d.applications.empty())
        for (const auto& a : d.applications)
            out += "- №" + std::to_string(a["id"].asInt64()) + " " +
                   str(a, "type_title") + " — статус: " +
                   std::string(models::applicationStatusTitle(
                       str(a, "status"))) +
                   "\n";
    else
        out += "- нет\n";

    out += "\nБлижайшие события кампуса:\n";
    if (d.campus.isArray() && !d.campus.empty())
        for (const auto& n : d.campus)
        {
            out += "- " + mskDateTime(str(n, "event_at")) + " " +
                   str(n, "title");
            if (!str(n, "location").empty())
                out += ", " + str(n, "location");
            out += "\n";
        }
    else
        out += "- нет\n";
    return out;
}

Json::Value AssistantService::buildRequest(const Config& cfg,
                                           const std::string& contextText,
                                           const Json::Value& history,
                                           const std::string& message)
{
    Json::Value req(Json::objectValue);
    req["model"] = cfg.model;
    req["max_tokens"] = kMaxTokens;
    req["temperature"] = 0.3;
    Json::Value msgs(Json::arrayValue);
    Json::Value sys(Json::objectValue);
    sys["role"] = "system";
    sys["content"] = cfg.systemPrompt + "\n\n" + contextText;
    msgs.append(std::move(sys));
    if (history.isArray())
        for (const auto& h : history)
        {
            Json::Value m(Json::objectValue);
            m["role"] = h["role"].asString();
            m["content"] = h["content"].asString();
            msgs.append(std::move(m));
        }
    Json::Value user(Json::objectValue);
    user["role"] = "user";
    user["content"] = message;
    msgs.append(std::move(user));
    req["messages"] = std::move(msgs);
    return req;
}

std::optional<std::string> AssistantService::parseReply(int httpStatus,
                                                        const std::string& body)
{
    if (httpStatus != 200)
        return std::nullopt;
    Json::Value root;
    Json::CharReaderBuilder b;
    std::string errs;
    std::unique_ptr<Json::CharReader> reader(b.newCharReader());
    if (!reader->parse(body.data(), body.data() + body.size(), &root, &errs) ||
        !root.isObject())
        return std::nullopt;
    const auto& choices = root["choices"];
    if (!choices.isArray() || choices.empty())
        return std::nullopt;
    const auto& content = choices[0]["message"]["content"];
    if (!content.isString())
        return std::nullopt;
    std::string text = content.asString();
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return std::nullopt;
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// --- Транспорт -------------------------------------------------------------------

AssistantService::Transport AssistantService::httpTransport(
    trantor::EventLoop* loop)
{
    return [loop](const Config& cfg, const Json::Value& payload, ReplyCb cb) {
        // URL → origin (scheme://host[:port]) + path.
        const auto scheme = cfg.url.find("://");
        if (scheme == std::string::npos)
        {
            LOG_WARN << "ассистент: ASSISTANT_API_URL без схемы";
            cb(std::nullopt);
            return;
        }
        const auto slash = cfg.url.find('/', scheme + 3);
        const std::string origin =
            slash == std::string::npos ? cfg.url : cfg.url.substr(0, slash);
        const std::string path =
            slash == std::string::npos ? "/" : cfg.url.substr(slash);

        auto client = HttpClient::newHttpClient(origin, loop);
        auto req = HttpRequest::newHttpRequest();
        req->setMethod(Post);
        req->setPath(path);
        // Схема Api-Key (не Bearer) — YandexGPT API-ключ сервисного аккаунта.
        req->addHeader("Authorization", "Api-Key " + cfg.key);
        req->setContentTypeCode(CT_APPLICATION_JSON);
        Json::StreamWriterBuilder w;
        w["indentation"] = "";
        w["emitUTF8"] = true;
        req->setBody(Json::writeString(w, payload));

        // client захвачен в колбэк — живёт до ответа/таймаута.
        client->sendRequest(
            req,
            [cb, client](ReqResult result, const HttpResponsePtr& resp) {
                if (result != ReqResult::Ok || !resp)
                {
                    LOG_WARN << "ассистент: LLM недоступен (ReqResult "
                             << static_cast<int>(result) << ")";
                    cb(std::nullopt);
                    return;
                }
                const int status = static_cast<int>(resp->statusCode());
                auto reply = parseReply(status, std::string(resp->body()));
                if (!reply)
                    // Тело апстрима НЕ логируем: в нём id каталога/облака.
                    LOG_WARN << "ассистент: LLM ответил статусом " << status
                             << " без пригодного ответа";
                cb(std::move(reply));
            },
            kTimeoutSec);
    };
}

// --- Операции ----------------------------------------------------------------------

namespace {
const std::string kDeadlinesSql =
    "SELECT title, kind, due_at FROM deadlines "
    "WHERE (user_id = $1 OR (user_id IS NULL AND group_id = $2)) "
    "AND due_at > $3 ORDER BY due_at LIMIT 10";

const std::string kHistorySql =
    "SELECT id, role, content, created_at FROM assistant_messages "
    "WHERE user_id = $1 ORDER BY id DESC LIMIT $2";

const std::string kInsertPairSql =
    "INSERT INTO assistant_messages(user_id, role, content) VALUES "
    "($1, 'user', $2), ($3, 'assistant', $4)";

// Сборщик параллельных источников: cb стреляет ровно один раз — когда
// отработали все, либо при первой ошибке (fail-closed).
struct Gather
{
    AssistantService::ContextData data;
    AssistantService::ContextCb cb;
    std::atomic<int> pending{0};
    std::atomic<bool> failed{false};
    std::mutex mu;

    void done(bool ok)
    {
        if (!ok && !failed.exchange(true))
        {
            cb(false, AssistantService::ContextData{});
            return;
        }
        if (--pending == 0 && !failed.load())
            cb(true, std::move(data));
    }
};
}  // namespace

void AssistantService::collectContext(orm::DbClientPtr db,
                                      const SessionInfo& session,
                                      const std::string& today,
                                      ContextCb cb)
{
    const int64_t userId = session.userId;
    ScheduleService::groupOfUser(
        db, userId,
        [db, session, today, userId, cb](ScheduleService::GroupLookup lookup,
                                         std::optional<int64_t> groupId) {
            if (lookup == ScheduleService::GroupLookup::Error)
            {
                cb(false, ContextData{});
                return;
            }
            auto g = std::make_shared<Gather>();
            g->cb = cb;
            g->data.today = today;
            g->data.events = Json::Value(Json::arrayValue);
            g->data.exams = Json::Value(Json::arrayValue);
            g->data.deadlines = Json::Value(Json::arrayValue);
            g->data.applications = Json::Value(Json::arrayValue);
            g->data.campus = Json::Value(Json::arrayValue);
            const bool hasGroup = groupId.has_value();
            g->pending = hasGroup ? 5 : 4;
            const std::string now = nowIso();

            if (hasGroup)
                ScheduleService::eventsOnRange(
                    db, *groupId, today + "T00:00:00Z",
                    ScheduleService::addDays(today, 2) + "T00:00:00Z",
                    [g](bool ok, Json::Value events) {
                        if (ok)
                        {
                            std::lock_guard<std::mutex> l(g->mu);
                            g->data.events = std::move(events);
                        }
                        g->done(ok);
                    });

            GradesService::exams(db, userId, [g](GradesService::Result r) {
                if (r.ok)
                {
                    std::lock_guard<std::mutex> l(g->mu);
                    g->data.exams = std::move(r.body["exams"]);
                }
                g->done(r.ok);
            });

            ApplicationsService::list(
                db, userId, std::nullopt,
                [g](ApplicationsService::Result r) {
                    if (r.ok)
                    {
                        Json::Value active(Json::arrayValue);
                        for (const auto& a : r.body["applications"])
                            if (DashboardService::isActiveApplicationStatus(
                                    a["status"].asString()))
                                active.append(a);
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.applications = std::move(active);
                    }
                    g->done(r.ok);
                });

            *db << kDeadlinesSql << userId << groupId.value_or(-1) << now >>
                [g](const orm::Result& rows) {
                    Json::Value arr(Json::arrayValue);
                    for (const auto& r : rows)
                    {
                        Json::Value x(Json::objectValue);
                        x["title"] = r["title"].as<std::string>();
                        x["kind"] = r["kind"].as<std::string>();
                        x["due_at"] = r["due_at"].as<std::string>();
                        arr.append(std::move(x));
                    }
                    {
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.deadlines = std::move(arr);
                    }
                    g->done(true);
                } >>
                [g, userId](const DrogonDbException& e) {
                    LOG_ERROR << "AssistantService: дедлайны(" << userId
                              << "): " << e.base().what();
                    g->done(false);
                };

            NewsService::feed(
                db, session, NewsService::Query{},
                [g, now](NewsService::FeedResult r) {
                    const bool ok = r.status == NewsService::FeedStatus::Ok;
                    if (ok)
                    {
                        // Лента — по published_at; в контекст — ближайшие
                        // 5 будущих событий по времени события.
                        std::vector<Json::Value> upcoming;
                        for (const auto& n : r.body["news"])
                            if (n["event_at"].isString() &&
                                n["event_at"].asString() >= now)
                                upcoming.push_back(n);
                        std::sort(upcoming.begin(), upcoming.end(),
                                  [](const Json::Value& a,
                                     const Json::Value& b) {
                                      return a["event_at"].asString() <
                                             b["event_at"].asString();
                                  });
                        Json::Value arr(Json::arrayValue);
                        for (std::size_t i = 0;
                             i < upcoming.size() && i < 5; ++i)
                            arr.append(upcoming[i]);
                        std::lock_guard<std::mutex> l(g->mu);
                        g->data.campus = std::move(arr);
                    }
                    g->done(ok);
                });
        });
}

void AssistantService::history(orm::DbClientPtr db,
                               int64_t userId,
                               int limit,
                               HistoryCb cb)
{
    *db << kHistorySql << userId << limit >>
        [cb](const orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            // DESC LIMIT → разворот: клиенту и в промт — по возрастанию.
            for (auto it = rows.rbegin(); it != rows.rend(); ++it)
            {
                const auto& r = *it;
                Json::Value m(Json::objectValue);
                m["id"] = r["id"].as<int64_t>();
                m["role"] = r["role"].as<std::string>();
                m["content"] = r["content"].as<std::string>();
                m["created_at"] = r["created_at"].as<std::string>();
                arr.append(std::move(m));
            }
            cb(true, std::move(arr));
        } >>
        [cb, userId](const DrogonDbException& e) {
            LOG_ERROR << "AssistantService::history(" << userId
                      << "): " << e.base().what();
            cb(false, Json::Value(Json::arrayValue));
        };
}

void AssistantService::chat(orm::DbClientPtr db,
                            const SessionInfo& session,
                            const std::string& message,
                            const Config& cfg,
                            Transport transport,
                            ChatCb cb)
{
    if (!cfg.configured())
    {
        LOG_WARN << "ассистент не настроен (ASSISTANT_* в окружении)";
        cb(ChatResult{Status::Unavailable, {}});
        return;
    }
    const int64_t userId = session.userId;
    collectContext(
        db, session, ScheduleService::todayUtc(),
        [db, userId, message, cfg, transport, cb](bool ok, ContextData data) {
            if (!ok)
            {
                cb(ChatResult{Status::Error, {}});
                return;
            }
            const std::string context = renderContext(data);
            history(
                db, userId, kHistoryForPrompt,
                [db, userId, message, cfg, transport, cb,
                 context](bool hok, Json::Value hist) {
                    if (!hok)
                    {
                        cb(ChatResult{Status::Error, {}});
                        return;
                    }
                    const Json::Value payload =
                        buildRequest(cfg, context, hist, message);
                    transport(
                        cfg, payload,
                        [db, userId, message, cb](
                            std::optional<std::string> reply) {
                            if (!reply)
                            {
                                cb(ChatResult{Status::Unavailable, {}});
                                return;
                            }
                            const std::string text = *reply;
                            *db << kInsertPairSql << userId << message
                                << userId << text >>
                                [cb, text](const orm::Result&) {
                                    cb(ChatResult{Status::Ok, text});
                                } >>
                                [cb, userId](const DrogonDbException& e) {
                                    LOG_ERROR << "AssistantService: запись "
                                                 "истории("
                                              << userId
                                              << "): " << e.base().what();
                                    cb(ChatResult{Status::Error, {}});
                                };
                        });
                });
        });
}

}  // namespace para
