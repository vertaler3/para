#include "services/ApplicationsService.h"

#include <atomic>
#include <memory>
#include <string>

#include <drogon/drogon.h>

#include "models/Applications.h"
#include "services/AppWorkflow.h"

using namespace drogon;
using namespace drogon::orm;

namespace para
{
namespace
{

// Заявка — общий вид строки для POST/PUT-ответа, списка и карточки.
// type_title — из каталога-константы (03 §4.5.2); payload — разобранный
// JSON (в БД пишется только валидированным; защитный fallback — строкой).
Json::Value appJson(const Row& r)
{
    Json::Value a(Json::objectValue);
    a["id"] = r["id"].as<int64_t>();
    const std::string type = r["type"].as<std::string>();
    a["type"] = type;
    a["type_title"] = std::string(models::applicationTypeTitle(type));
    a["status"] = r["status"].as<std::string>();
    const std::string raw = r["payload_json"].as<std::string>();
    Json::Value payload;
    Json::Reader reader;  // jsoncpp 1.y — как в SettingsService
    if (reader.parse(raw, payload))
        a["payload"] = std::move(payload);
    else
        a["payload"] = raw;
    a["created_at"] = r["created_at"].as<std::string>();
    a["updated_at"] = r["updated_at"].as<std::string>();
    return a;
}

// Событие таймлайна (application_events 02 §4.1): from_status/comment/
// author_id — nullable по DDL → JSON null.
Json::Value eventJson(const Row& r)
{
    Json::Value e(Json::objectValue);
    e["id"] = r["id"].as<int64_t>();
    e["application_id"] = r["application_id"].as<int64_t>();
    if (r["from_status"].isNull())
        e["from_status"] = Json::Value();
    else
        e["from_status"] = r["from_status"].as<std::string>();
    e["to_status"] = r["to_status"].as<std::string>();
    if (r["comment"].isNull())
        e["comment"] = Json::Value();
    else
        e["comment"] = r["comment"].as<std::string>();
    if (r["author_id"].isNull())
        e["author_id"] = Json::Value();
    else
        e["author_id"] = r["author_id"].as<int64_t>();
    e["at"] = r["at"].as<std::string>();
    return e;
}

// Строка с валидным JSON (payload_json; SettingsService::isValidJsonString).
bool isValidJsonString(const std::string& s)
{
    Json::Value parsed;
    Json::Reader reader;
    return reader.parse(s, parsed);
}

const std::string kAppColumns =
    "id, type, status, payload_json, created_at, updated_at";

// id последней вставленной строки на соединении — читается ВНУТРИ
// транзакции (соединение закреплено, межзапросной гонки нет; AppWorkflow.cpp).
const std::string kLastRowIdSql = "SELECT last_insert_rowid() AS id";

const std::string kSelectAppSql =
    "SELECT " + kAppColumns + " FROM applications WHERE id = $1";

const std::string kSelectNewAppSql =
    "SELECT " + kAppColumns +
    " FROM applications WHERE id = last_insert_rowid()";

const std::string kListSql =
    "SELECT " + kAppColumns +
    " FROM applications WHERE user_id = $1 ORDER BY created_at DESC, id DESC";

const std::string kListByStatusSql =
    "SELECT " + kAppColumns +
    " FROM applications WHERE user_id = $1 AND status = $2"
    " ORDER BY created_at DESC, id DESC";

const std::string kTimelineSql =
    "SELECT id, application_id, from_status, to_status, comment, author_id, at"
    " FROM application_events WHERE application_id = $1 ORDER BY at, id";

const std::string kSelectEventSql =
    "SELECT id, application_id, from_status, to_status, comment, author_id, at"
    " FROM application_events WHERE id = last_insert_rowid()";

const std::string kStatusSql =
    "SELECT status FROM applications WHERE id = $1";

// Событие подачи (fixtures.sql: from NULL, to 'submitted', автор — студент).
const std::string kInsertSubmitEventSql =
    "INSERT INTO application_events"
    " (application_id, from_status, to_status, comment, author_id)"
    " VALUES ($1, NULL, 'submitted', NULL, $2)";

// Сообщение мини-диалога: from=to=фактический статус, прочитанный ВНУТРИ
// той же транзакции (см. insertPlainMessageEvent) — гонка с параллельной
// сменой статуса не оставляет устаревших from/to в таймлайне.
// ВАЖНО (поймано тестами шага 2): в SQLite $N — ИМЯ параметра, индекс
// назначается по первому вхождению в тексте SQL (в отличие от ?N), а drogon
// биндит аргументы по порядку (i+1). Порядок $N в тексте обязан совпадать с
// порядком биндинга — иначе значения уходят в чужие слоты молча.
const std::string kInsertMessageEventSql =
    "INSERT INTO application_events"
    " (application_id, from_status, to_status, comment, author_id)"
    " VALUES ($1, $2, $3, $4, $5)";

// PUT черновика: payload только при status='draft' (02 §5.2); updated_at —
// формат DDL (очередь сотрудника сортируется по idx_applications_queue).
const std::string kUpdateDraftSql =
    "UPDATE applications SET payload_json = $1,"
    " updated_at = strftime('%Y-%m-%dT%H:%M:%SZ','now')"
    " WHERE id = $2 AND status = 'draft'";

// Событие таймлайна по PK — для тела 201 messages (после Applied apply()
// eventId известен точно; «последнее событие» на гонках не используется).
void fetchEventById(const drogon::orm::DbClientPtr& db,
                    int64_t eventId,
                    std::function<void(ApplicationsService::MessageResult)> cb)
{
    *db << "SELECT id, application_id, from_status, to_status, comment,"
           " author_id, at FROM application_events WHERE id = $1"
        << eventId >>
        [cb, eventId](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                // После commit событие есть; исчезновение — аномалия
                // (ON DELETE CASCADE при удалении заявки в гонке).
                LOG_ERROR << "ApplicationsService: событие " << eventId
                          << " не найдено после записи";
                cb(ApplicationsService::MessageResult{
                    ApplicationsService::Message::Error, Json::Value()});
                return;
            }
            cb(ApplicationsService::MessageResult{
                ApplicationsService::Message::Ok, eventJson(rows[0])});
        } >>
        [cb, eventId](const DrogonDbException& e) {
            LOG_ERROR << "ApplicationsService: fetch event " << eventId
                      << ": " << e.base().what();
            cb(ApplicationsService::MessageResult{
                ApplicationsService::Message::Error, Json::Value()});
        };
}

// Обычное событие сообщения (без смены статуса) — в транзакции: SELECT
// статуса заявки (строка исчезла → NotFound) → INSERT..VALUES события
// (from=to=этот статус) → SELECT строки события по last_insert_rowid() →
// commit. Транзакция закрепляет единственное соединение клиента — между
// SELECT и INSERT другой запрос не вклинится. В тело ответа добавляется
// "application_status" — итоговый статус заявки (правки по гейту Э5
// раунд 1: возврат/неизменность статуса читается прямо в ответе, без
// второго GET). Колбэк стреляет ровно один раз (commit-callback /
// error-ветки), как в AppWorkflow::apply.
void insertPlainMessageEvent(const drogon::orm::DbClientPtr& db,
                             int64_t applicationId,
                             const std::string& text,
                             int64_t authorId,
                             std::function<void(ApplicationsService::MessageResult)> cb)
{
    db->newTransactionAsync([applicationId, text, authorId,
                             cb](const std::shared_ptr<Transaction>& tx) {
        if (!tx)
        {
            LOG_ERROR << "ApplicationsService: транзакция недоступна (timeout)";
            cb(ApplicationsService::MessageResult{
                ApplicationsService::Message::Error, Json::Value()});
            return;
        }
        auto once = std::make_shared<std::atomic_bool>(false);
        auto body = std::make_shared<Json::Value>(Json::Value());
        auto fire = [cb, once](bool ok, Json::Value b) {
            bool expected = false;
            if (!once->compare_exchange_strong(expected, true))
                return;
            if (!ok)
                LOG_WARN << "ApplicationsService: транзакция откатывается";
            cb(ApplicationsService::MessageResult{
                ok ? ApplicationsService::Message::Ok
                   : ApplicationsService::Message::Error,
                std::move(b)});
        };

        // 1. Фактический статус на момент записи (внутри транзакции).
        *tx << kStatusSql << applicationId >>
            [tx, applicationId, text, authorId, body,
             fire](const drogon::orm::Result& rows) {
                if (rows.empty())
                {
                    // Заявка исчезла (гонка удаления) → NotFound (404).
                    LOG_WARN << "ApplicationsService: заявка " << applicationId
                             << " исчезла при записи сообщения";
                    fire(false, Json::Value());
                    tx->rollback();
                    return;
                }
                const std::string status =
                    rows[0]["status"].as<std::string>();
                // 2. Событие from=to=этот статус (С7: текст — комментарий).
                *tx << kInsertMessageEventSql << applicationId << status
                    << status << text << authorId >>
                    [tx, body, status,
                     fire](const drogon::orm::Result&) {
                        // 3. Строка события (id = last_insert_rowid() этого
                        //    INSERT — между ними на соединении транзакции
                        //    ничего не вклинится).
                        *tx << kSelectEventSql >>
                            [tx, body, status,
                             fire](const drogon::orm::Result& evRows) {
                                if (evRows.empty())
                                {
                                    LOG_ERROR << "ApplicationsService: "
                                                 "событие не прочитано после "
                                                 "вставки";
                                    fire(false, Json::Value());
                                    tx->rollback();
                                    return;
                                }
                                *body = eventJson(evRows[0]);
                                // Итоговый статус заявки в ответе (правки
                                // по гейту Э5 раунд 1) — то же поле, что
                                // в need_info-ветке sendMessage.
                                (*body)["application_status"] = status;
                            } >>
                            [tx, fire](const DrogonDbException& e) {
                                LOG_ERROR << "ApplicationsService: select "
                                             "event: "
                                          << e.base().what();
                                fire(false, Json::Value());
                                tx->rollback();
                            };
                    } >>
                    [tx, fire](const DrogonDbException& e) {
                        LOG_ERROR << "ApplicationsService: insert message: "
                                  << e.base().what();
                        fire(false, Json::Value());
                        tx->rollback();
                    };
            } >>
            [tx, fire](const DrogonDbException& e) {
                LOG_ERROR << "ApplicationsService: select status: "
                          << e.base().what();
                fire(false, Json::Value());
                tx->rollback();
            };

        tx->setCommitCallback([fire, body](bool committed) {
            fire(committed, committed ? *body : Json::Value());
        });
    });
}

// ============================== Э5 шаг 3 — staff ===========================

// Очередь (02 §5.2: query status, type, overdue=1; сортировка по
// updated_at). Одна параметризованная SQL с «filters-or-empty»: текст $N
// идёт в порядке биндинга ($1,$2,$3 — правило из хода шага 2). overdue —
// 3 дня ожидания в активном статусе (см. заголовок метода). JOIN
// profiles/groups — С8.3 (карточка сотрудника показывает студента); LEFT —
// заявка без профиля не пропадает из очереди. «Горящие» считаются и без
// фильтра overdue=1 (колонка overdue — бейдж в строках очереди, 03 §4.8).
const std::string kQueueSql =
    "SELECT a.id, a.type, a.status, a.payload_json, a.created_at,"
    " a.updated_at, p.fio, g.name AS group_name,"
    " (a.updated_at < strftime('%Y-%m-%dT%H:%M:%SZ','now','-3 days')"
    "  AND a.status IN ('submitted','accepted','in_progress','need_info'))"
    "  AS overdue"
    " FROM applications a"
    " LEFT JOIN profiles p ON p.user_id = a.user_id"
    " LEFT JOIN groups g ON g.id = p.group_id"
    " WHERE ($1 = '' OR a.status = $1)"
    "   AND ($2 = '' OR a.type = $2)"
    "   AND ($3 = '' OR ("
    "         a.updated_at < strftime('%Y-%m-%dT%H:%M:%SZ','now','-3 days')"
    "         AND a.status IN ('submitted','accepted','in_progress',"
    "                          'need_info')))"
    " ORDER BY a.updated_at ASC, a.id ASC";

// Владелец заявки для уведомления — ВНУТРИ транзакции после UPDATE
// (соединение закреплено; user_id NOT NULL по DDL).
const std::string kSelectOwnerSql =
    "SELECT user_id, type FROM applications WHERE id = $1";

// Уведомление автору заявки (02 §4.1 notifications: type 'application';
// deep_link — тап → таймлайн заявки, 4.5; title — «Тип: Статус» по меткам
// models). Порядок $N в тексте = порядку биндинга.
const std::string kInsertNotificationSql =
    "INSERT INTO notifications (user_id, type, title, body, deep_link)"
    " VALUES ($1, 'application', $2, $3, $4)";

Json::Value queueJson(const Row& r)
{
    Json::Value a = appJson(r);  // id/type/type_title/status/payload/даты
    Json::Value owner(Json::objectValue);
    if (r["fio"].isNull())
        owner["fio"] = Json::Value();
    else
        owner["fio"] = r["fio"].as<std::string>();
    if (r["group_name"].isNull())
        owner["group"] = Json::Value();
    else
        owner["group"] = r["group_name"].as<std::string>();
    a["owner"] = std::move(owner);
    a["overdue"] = r["overdue"].as<int64_t>() != 0;
    return a;
}

}  // namespace

void ApplicationsService::queue(drogon::orm::DbClientPtr db,
                                const QueueFilters& filters,
                                ResultCb cb)
{
    // Пустая строка = «без фильтра» (filters-or-empty в kQueueSql).
    const std::string status = filters.status.value_or("");
    const std::string type = filters.type.value_or("");
    const std::string overdue = filters.overdueOnly ? "1" : "";
    *db << kQueueSql << status << type << overdue >>
        [cb](const drogon::orm::Result& rows) {
            Json::Value arr(Json::arrayValue);
            for (const auto& r : rows)
                arr.append(queueJson(r));
            Json::Value body;
            body["applications"] = std::move(arr);
            cb(Result{true, std::move(body)});
        } >>
        [cb](const DrogonDbException& e) {
            LOG_ERROR << "ApplicationsService::queue: " << e.base().what();
            cb(Result{false, Json::Value()});  // fail-closed → 500
        };
}

void ApplicationsService::staffChangeStatus(
    drogon::orm::DbClientPtr db,
    int64_t applicationId,
    const std::string& toStatus,
    const std::optional<std::string>& comment,
    int64_t staffUserId,
    StatusChangeCb cb)
{
    // Хук: автоуведомление автору заявки В ТОЙ ЖЕ транзакции (05 §3 Э5 п.3:
    // «UPDATE + событие таймлайна + INSERT в notifications»); fail() хука —
    // откат всего перехода и Error (см. AppWorkflow::AfterEventTx).
    AppWorkflow::AfterEventTx notify =
        [applicationId, toStatus,
         comment](const std::shared_ptr<Transaction>& tx,
                  const std::function<void()>& fail) {
            *tx << kSelectOwnerSql << applicationId >>
                [tx, applicationId, toStatus, comment,
                 fail](const drogon::orm::Result& rows) {
                    if (rows.empty())
                    {
                        // Заявка удалена в гонке — некому уведомлять.
                        // fail() = fire(Error) + rollback всей транзакции.
                        LOG_WARN << "ApplicationsService: заявка "
                                 << applicationId << " исчезла перед "
                                                    "уведомлением";
                        fail();
                        return;
                    }
                    const int64_t ownerId =
                        rows[0]["user_id"].as<int64_t>();
                    const std::string type = rows[0]["type"].as<std::string>();
                    const std::string title =
                        std::string(models::applicationTypeTitle(type)) +
                        ": " + std::string(models::applicationStatusTitle(toStatus));
                    const std::string deepLink =
                        "/applications/" + std::to_string(applicationId);
                    *tx << kInsertNotificationSql << ownerId << title
                        << comment << deepLink >>
                        [](const drogon::orm::Result&) {
                            // commit — при уничтожении tx (последний
                            // держатель — этот колбэк).
                        } >>
                        [fail](const DrogonDbException& e) {
                            LOG_ERROR << "ApplicationsService: insert"
                                         " notification: "
                                      << e.base().what();
                            fail();
                        };
                } >>
                [fail](const DrogonDbException& e) {
                    LOG_ERROR << "ApplicationsService: select owner: "
                              << e.base().what();
                    fail();
                };
        };

    AppWorkflow::apply(
        db, applicationId, toStatus, comment, staffUserId,
        [db, applicationId,
         cb](AppWorkflow::ApplyResult r) {
            switch (r.outcome)
            {
                case AppWorkflow::Outcome::Applied:
                    // Карточка в новом статусе (по PK — после Apply).
                    *db << kSelectAppSql << applicationId >>
                        [cb](const drogon::orm::Result& rows) {
                            if (rows.empty())
                            {
                                cb(StatusChangeResult{
                                    StatusChange::NotFound, Json::Value()});
                                return;
                            }
                            cb(StatusChangeResult{StatusChange::Ok,
                                                  appJson(rows[0])});
                        } >>
                        [cb](const DrogonDbException& e) {
                            LOG_ERROR << "ApplicationsService::"
                                         "staffChangeStatus: select: "
                                      << e.base().what();
                            cb(StatusChangeResult{StatusChange::Error,
                                                  Json::Value()});
                        };
                    return;
                case AppWorkflow::Outcome::InvalidTransition:
                    cb(StatusChangeResult{StatusChange::InvalidTransition,
                                          Json::Value()});
                    return;
                case AppWorkflow::Outcome::NotFound:
                    cb(StatusChangeResult{StatusChange::NotFound,
                                          Json::Value()});
                    return;
                case AppWorkflow::Outcome::Error:
                    cb(StatusChangeResult{StatusChange::Error,
                                          Json::Value()});
                    return;
            }
        },
        std::move(notify));
}

Json::Value ApplicationsService::catalog()
{
    Json::Value arr(Json::arrayValue);
    for (const auto& t : models::kApplicationTypes)
    {
        Json::Value item(Json::objectValue);
        item["key"] = std::string(t.key);
        item["title"] = std::string(t.title);
        arr.append(std::move(item));
    }
    Json::Value body;
    body["types"] = std::move(arr);
    return body;
}

ApplicationsService::CreateSpec ApplicationsService::parseCreate(
    const Json::Value& body)
{
    CreateSpec spec;
    if (!body.isMember("type") || !body["type"].isString())
    {
        spec.errorCode = "bad_request";
        spec.errorMessage = "type — ожидается ключ каталога типов";
        return spec;
    }
    const std::string type = body["type"].asString();
    if (!models::isKnownApplicationType(type))
    {
        spec.errorCode = "bad_request";
        spec.errorMessage = "type — неизвестный ключ каталога типов";
        return spec;
    }
    spec.type = type;

    // payload_json опционален → '{}' (DEFAULT DDL 02 §4.1).
    if (body.isMember("payload_json"))
    {
        if (!body["payload_json"].isString() ||
            !isValidJsonString(body["payload_json"].asString()))
        {
            spec.errorCode = "bad_request";
            spec.errorMessage = "payload_json — ожидается валидный JSON";
            return spec;
        }
        spec.payloadJson = body["payload_json"].asString();
    }
    else
    {
        spec.payloadJson = "{}";
    }

    if (body.isMember("submit"))
    {
        if (!body["submit"].isBool())
        {
            spec.errorCode = "bad_request";
            spec.errorMessage = "submit — ожидается true/false";
            return spec;
        }
        spec.submit = body["submit"].asBool();
    }
    spec.ok = true;
    return spec;
}

ApplicationsService::TextSpec ApplicationsService::parsePayloadJson(
    const Json::Value& body)
{
    TextSpec spec;
    if (!body.isMember("payload_json") || !body["payload_json"].isString() ||
        !isValidJsonString(body["payload_json"].asString()))
    {
        spec.errorCode = "bad_request";
        spec.errorMessage = "payload_json — ожидается валидный JSON";
        return spec;
    }
    spec.text = body["payload_json"].asString();
    spec.ok = true;
    return spec;
}

ApplicationsService::TextSpec ApplicationsService::parseMessageText(
    const Json::Value& body)
{
    TextSpec spec;
    if (!body.isMember("text") || !body["text"].isString())
    {
        spec.errorCode = "bad_request";
        spec.errorMessage = "text — ожидается строка";
        return spec;
    }
    const std::string text = body["text"].asString();
    // Пробелы по краям обрезаем; «пустой» ответ диалогом не считается.
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        spec.errorCode = "bad_request";
        spec.errorMessage = "text — ожидается непустое сообщение";
        return spec;
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    spec.text = text.substr(first, last - first + 1);
    spec.ok = true;
    return spec;
}

void ApplicationsService::create(drogon::orm::DbClientPtr db,
                                 int64_t userId,
                                 const CreateSpec& spec,
                                 ResultCb cb)
{
    const std::string status = spec.submit ? "submitted" : "draft";
    db->newTransactionAsync(
        [userId, spec, status,
         cb](const std::shared_ptr<Transaction>& tx) {
            if (!tx)
            {
                LOG_ERROR << "ApplicationsService::create: транзакция "
                             "недоступна (timeout)";
                cb(Result{false, Json::Value()});
                return;
            }
            auto once = std::make_shared<std::atomic_bool>(false);
            auto body = std::make_shared<Json::Value>(Json::Value());
            auto fire = [cb, once](bool ok, Json::Value b) {
                bool expected = false;
                if (!once->compare_exchange_strong(expected, true))
                    return;
                if (!ok)
                    LOG_WARN << "ApplicationsService::create: транзакция "
                                "откатывается";
                cb(Result{ok, std::move(b)});
            };

            // 1. INSERT заявки (draft или submitted).
            *tx << "INSERT INTO applications (user_id, type, status,"
                   " payload_json) VALUES ($1, $2, $3, $4)"
                << userId << spec.type << status << spec.payloadJson >>
                [tx, userId, status, body, fire](const drogon::orm::Result&) {
                    // 2. Строка новой заявки: id (last_insert_rowid() на
                    //    соединении транзакции) + DDL-даты. ДО вставки
                    //    события — иначе rowid указывает на событие.
                    *tx << kSelectNewAppSql >>
                        [tx, userId, status, body,
                         fire](const drogon::orm::Result& rows) {
                            if (rows.empty())
                            {
                                LOG_ERROR << "ApplicationsService::create: "
                                             "новая строка не прочитана";
                                fire(false, Json::Value());
                                tx->rollback();
                                return;
                            }
                            *body = appJson(rows[0]);
                            if (status != "submitted")
                                return;  // черновик — без события (fixtures)
                            // 3. Событие подачи NULL→submitted (С2.4 —
                            //    первая точка таймлайна; fixtures.sql).
                            const int64_t appId = (*body)["id"].as<int64_t>();
                            *tx << kInsertSubmitEventSql << appId << userId >>
                                [](const drogon::orm::Result&) {
                                    // commit — при уничтожении tx.
                                } >>
                                [tx, fire](const DrogonDbException& e) {
                                    LOG_ERROR
                                        << "ApplicationsService::create: "
                                           "insert event: "
                                        << e.base().what();
                                    fire(false, Json::Value());
                                    tx->rollback();
                                };
                        } >>
                        [tx, fire](const DrogonDbException& e) {
                            LOG_ERROR << "ApplicationsService::create: "
                                         "select new: "
                                      << e.base().what();
                            fire(false, Json::Value());
                            tx->rollback();
                        };
                } >>
                [tx, fire](const DrogonDbException& e) {
                    LOG_ERROR << "ApplicationsService::create: insert: "
                              << e.base().what();
                    fire(false, Json::Value());
                    tx->rollback();
                };

            tx->setCommitCallback([fire, body](bool committed) {
                fire(committed, committed ? *body : Json::Value());
            });
        });
}

void ApplicationsService::list(drogon::orm::DbClientPtr db,
                               int64_t userId,
                               const std::optional<std::string>& statusFilter,
                               ResultCb cb)
{
    // Колбэк-тело общее для обеих цепочек (копируется — стреляет одна).
    auto onRows = [cb](const drogon::orm::Result& rows) {
        Json::Value arr(Json::arrayValue);
        for (const auto& r : rows)
            arr.append(appJson(r));
        Json::Value body;
        body["applications"] = std::move(arr);
        cb(Result{true, std::move(body)});
    };
    auto onError = [cb, userId](const DrogonDbException& e) {
        LOG_ERROR << "ApplicationsService::list(" << userId
                  << "): " << e.base().what();
        cb(Result{false, Json::Value()});  // fail-closed → 500
    };

    if (!statusFilter.has_value())
    {
        *db << kListSql << userId >> std::move(onRows) >> std::move(onError);
        return;
    }
    // Валидность значения проверил контроллер (словарь kApplicationStatuses).
    *db << kListByStatusSql << userId << *statusFilter >> std::move(onRows) >>
        std::move(onError);
}

void ApplicationsService::one(drogon::orm::DbClientPtr db,
                              int64_t applicationId,
                              OneCb cb)
{
    *db << kSelectAppSql << applicationId >>
        [db, applicationId, cb](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                // После Own guard'а строка исчезнуть не должна (гонка
                // удаления) — честный 404, не 500 и не чужие данные.
                cb(OneResult{One::NotFound, Json::Value()});
                return;
            }
            Json::Value card = appJson(rows[0]);
            *db << kTimelineSql << applicationId >>
                [card = std::move(card),
                 cb](const drogon::orm::Result& evRows) mutable {
                    Json::Value arr(Json::arrayValue);
                    for (const auto& r : evRows)
                        arr.append(eventJson(r));
                    card["timeline"] = std::move(arr);
                    cb(OneResult{One::Found, std::move(card)});
                } >>
                [cb, applicationId](const DrogonDbException& e) {
                    LOG_ERROR << "ApplicationsService::one(" << applicationId
                              << "): timeline: " << e.base().what();
                    cb(OneResult{One::Error, Json::Value()});
                };
        } >>
        [cb, applicationId](const DrogonDbException& e) {
            LOG_ERROR << "ApplicationsService::one(" << applicationId
                      << "): " << e.base().what();
            cb(OneResult{One::Error, Json::Value()});  // fail-closed → 500
        };
}

void ApplicationsService::updateDraft(drogon::orm::DbClientPtr db,
                                      int64_t applicationId,
                                      const std::string& payloadJson,
                                      UpdateCb cb)
{
    // Условный UPDATE: только draft (02 §5.2 «409 (статус ≠ draft)»);
    // guard владения уже отработал в контроллере (до проверки статуса).
    *db << kUpdateDraftSql << payloadJson << applicationId >>
        [db, applicationId, cb](const drogon::orm::Result& r) {
            if (r.affectedRows() == 1)
            {
                // Обновлено → прочитать карточку (payload/updated_at).
                *db << kSelectAppSql << applicationId >>
                    [cb](const drogon::orm::Result& rows) {
                        if (rows.empty())
                        {
                            cb(UpdateResult{Update::NotFound, Json::Value()});
                            return;
                        }
                        cb(UpdateResult{Update::Ok, appJson(rows[0])});
                    } >>
                    [cb, applicationId](const DrogonDbException& e) {
                        LOG_ERROR << "ApplicationsService::updateDraft("
                                  << applicationId << "): select: "
                                  << e.base().what();
                        cb(UpdateResult{Update::Error, Json::Value()});
                    };
                return;
            }
            // 0 строк: заявки нет ИЛИ статус ≠ draft — различить SELECT'ом.
            *db << kStatusSql << applicationId >>
                [cb](const drogon::orm::Result& rows) {
                    if (rows.empty())
                        cb(UpdateResult{Update::NotFound, Json::Value()});
                    else
                        cb(UpdateResult{Update::NotDraft, Json::Value()});
                } >>
                [cb, applicationId](const DrogonDbException& e) {
                    LOG_ERROR << "ApplicationsService::updateDraft("
                              << applicationId << "): status: "
                              << e.base().what();
                    cb(UpdateResult{Update::Error, Json::Value()});
                };
        } >>
        [cb, applicationId](const DrogonDbException& e) {
            LOG_ERROR << "ApplicationsService::updateDraft(" << applicationId
                      << "): update: " << e.base().what();
            cb(UpdateResult{Update::Error, Json::Value()});  // fail-closed
        };
}

void ApplicationsService::sendMessage(drogon::orm::DbClientPtr db,
                                      int64_t applicationId,
                                      const std::string& text,
                                      int64_t authorId,
                                      MessageCb cb)
{
    *db << kStatusSql << applicationId >>
        [db, applicationId, text, authorId,
         cb](const drogon::orm::Result& rows) {
            if (rows.empty())
            {
                cb(MessageResult{Message::NotFound, Json::Value()});
                return;
            }
            const std::string status = rows[0]["status"].as<std::string>();
            if (status != "need_info")
            {
                // Обычное сообщение (без смены статуса): application_status
                // добавляется внутри insertPlainMessageEvent из фактического
                // статуса транзакции (правки по гейту Э5 раунд 1).
                insertPlainMessageEvent(db, applicationId, text, authorId,
                                        std::move(cb));
                return;
            }
            // С7.6: ответ студента → «В работе» через автомат шага 1
            // (матрица need_info→in_progress); текст — комментарий события.
            AppWorkflow::apply(
                db, applicationId, "in_progress",
                std::optional<std::string>(text), authorId,
                [db, applicationId, text, authorId,
                 cb](AppWorkflow::ApplyResult r) mutable {
                    switch (r.outcome)
                    {
                        case AppWorkflow::Outcome::Applied:
                            // Возврат в «В работе» — в ответе и событием
                            // (need_info→in_progress), и полем
                            // application_status (гейт Э5 раунд 1).
                            fetchEventById(
                                db, r.eventId,
                                [cb](ApplicationsService::MessageResult m) mutable {
                                    if (m.status ==
                                        ApplicationsService::Message::Ok)
                                        m.body["application_status"] =
                                            "in_progress";
                                    cb(std::move(m));
                                });
                            return;
                        case AppWorkflow::Outcome::InvalidTransition:
                            // Гонка: статус уже ушли с need_info (проверка
                            // apply'а по фактическому статусу) — сообщение
                            // не теряем: событие от фактического статуса.
                            LOG_WARN << "ApplicationsService::sendMessage("
                                     << applicationId
                                     << "): need_info изменён параллельно";
                            insertPlainMessageEvent(db, applicationId, text,
                                                    authorId, std::move(cb));
                            return;
                        case AppWorkflow::Outcome::NotFound:
                            cb(MessageResult{Message::NotFound, Json::Value()});
                            return;
                        case AppWorkflow::Outcome::Error:
                            cb(MessageResult{Message::Error, Json::Value()});
                            return;
                    }
                });
        } >>
        [cb, applicationId](const DrogonDbException& e) {
            LOG_ERROR << "ApplicationsService::sendMessage(" << applicationId
                      << "): " << e.base().what();
            cb(MessageResult{Message::Error, Json::Value()});
        };
}

}  // namespace para
