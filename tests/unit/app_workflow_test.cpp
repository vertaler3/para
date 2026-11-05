// Юнит-тесты AppWorkflow — автомат статусов заявок (Э5 шаг 1; гайд BE §7;
// матрица 05 §3 Э5 п.1; DDL 02 §4.1 — applications/application_events,
// миграция 0003):
//  - ВСЕ 16 валидных переходов матрицы применяются к временной SQLite,
//    каждое применение пишет строку в application_events (from_status,
//    to_status, author_id; comment — как передали);
//  - С7.6: полный жизненный цикл одной заявки — need_info → in_progress
//    возвращает «В работе» после ответа студента (01 §3 С7), с комментарием
//    вопроса деканата;
//  - исчерпывающий перебор НЕвалидных пар словаря статусов (81 − 16 = 65,
//    включая названные в постановке submitted→issued и ready→in_progress,
//    и «issued → in_progress нет» 02 §5.2 поток 2) → InvalidTransition,
//    статус и таймлайн не меняются; неизвестные статусы — так же;
//  - несуществующая заявка → NotFound (404); ошибка БД → Error fail-closed
//    (500) — контроллер переводит по 02 §5.1;
//  - согласованность константы матрицы со словарём CHECK DDL.
// Схема — колонки/индексы миграции 0003 (с CHECK, FK не проверяются — это
// работа миграций). Данные: заявки по одной на статус (для перебора) и по
// одной на пару матрицы (для валидных переходов) — засевается ИЗ константы
// kApplicationStatusTransitions, чтобы тест не разошёлся с матрицей.
#include <chrono>
#include <filesystem>
#include <future>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include <doctest.hpp>

#include "models/Applications.h"
#include "services/AppWorkflow.h"

namespace fs = std::filesystem;
using drogon::orm::DrogonDbException;
using para::AppWorkflow;
using para::models::kApplicationStatusTransitions;
using para::models::kApplicationStatuses;

namespace
{

struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-appworkflow-" + std::to_string(stamp) + "-" +
                std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void exec(sqlite3* db, const std::string& sql)
{
    char* err = nullptr;
    REQUIRE(sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) == SQLITE_OK);
    if (err != nullptr)
    {
        FAIL(err);
        sqlite3_free(err);
    }
}

// Схема — колонки applications/application_events миграции 0003 (CHECK
// статусного словаря дословно из DDL 02 §4.1; FK опущены — их проверяют
// миграции, а не этот тест).
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withTables = true)
    {
        if (withTables)
            createSchema();
        db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);
    }

    void createSchema()
    {
        sqlite3* raw = nullptr;
        REQUIRE(sqlite3_open_v2(dbPath.c_str(),
                                &raw,
                                SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                nullptr) == SQLITE_OK);
        exec(raw,
             "CREATE TABLE applications ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  user_id INTEGER NOT NULL,"
             "  type TEXT NOT NULL,"
             "  status TEXT NOT NULL DEFAULT 'submitted'"
             "    CHECK (status IN ('draft','submitted','accepted','in_progress',"
             "                      'need_info','ready','issued','rejected','cancelled')),"
             "  payload_json TEXT NOT NULL DEFAULT '{}',"
             "  created_at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')),"
             "  updated_at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE INDEX idx_applications_user ON applications(user_id,"
             " status);"
             "CREATE INDEX idx_applications_queue ON applications(status,"
             " updated_at);"
             "CREATE TABLE application_events ("
             "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
             "  application_id INTEGER NOT NULL,"
             "  from_status TEXT,"
             "  to_status TEXT NOT NULL,"
             "  comment TEXT,"
             "  author_id INTEGER,"
             "  at TEXT NOT NULL DEFAULT"
             "    (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
             "CREATE INDEX idx_app_events ON application_events"
             " (application_id, at);");
        seedApplications(raw);
        sqlite3_close(raw);
    }

    // Заявка на каждый статус словаря: ids 1–9 (для перебора невалидных
    // пар) + заявка на каждую пару матрицы: ids 11–26 (для валидных
    // переходов — одна пара меняет одну заявку). user_id 4 (Дина, С2/С7).
    // Засев из констант models/Applications.h: тест и матрица не расходятся.
    // updated_at фиксирован в прошлом — тест проверяет его продление.
    void seedApplications(sqlite3* raw)
    {
        const std::string head =
            "INSERT INTO applications(id, user_id, type, status,"
            " payload_json, created_at, updated_at) VALUES (";
        for (std::size_t i = 0; i < kApplicationStatuses.size(); ++i)
        {
            // id 1..9 — по статусу словаря; статусы — constexpr-константы
            // из заголовка models (не пользовательский ввод).
            exec(raw,
                 head + std::to_string(i + 1) + ", 4, 'academic_certificate'"
                        ", '" + std::string(kApplicationStatuses[i]) +
                        "', '{}', '2026-11-01T12:00:00Z',"
                        " '2026-01-01T00:00:00Z');");
        }
        for (std::size_t i = 0; i < kApplicationStatusTransitions.size(); ++i)
        {
            const auto& t = kApplicationStatusTransitions[i];
            exec(raw,
                 head + std::to_string(11 + i) + ", 4, 'academic_certificate'"
                        ", '" + std::string(t.from) +
                        "', '{}', '2026-11-01T12:00:00Z',"
                        " '2026-01-01T00:00:00Z');");
        }
        REQUIRE(countApps(raw) ==
                kApplicationStatuses.size() +
                    kApplicationStatusTransitions.size());
    }

    static int countApps(sqlite3* raw)
    {
        int n = -1;
        char* err = nullptr;
        REQUIRE(sqlite3_exec(
                    raw,
                    "SELECT COUNT(*) FROM applications;",
                    [](void* out, int, char** cols, char**) {
                        *static_cast<int*>(out) = std::atoi(cols[0]);
                        return 0;
                    },
                    &n,
                    &err) == SQLITE_OK);
        REQUIRE(err == nullptr);
        return n;
    }
};

AppWorkflow::ApplyResult applyOf(const drogon::orm::DbClientPtr& db,
                                 int64_t appId,
                                 const std::string& to,
                                 const std::optional<std::string>& comment,
                                 int64_t author)
{
    std::promise<AppWorkflow::ApplyResult> p;
    auto f = p.get_future();
    AppWorkflow::apply(db, appId, to, comment, author,
                       [&p](AppWorkflow::ApplyResult r) {
                           p.set_value(std::move(r));
                       });
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

struct EventRow
{
    int64_t id = 0;
    int64_t applicationId = 0;
    bool hasFrom = false;
    std::string from;
    std::string to;
    bool hasComment = false;
    std::string comment;
    int64_t authorId = 0;
    std::string at;
};

// Таймлайн заявки — application_events по id (порядок записи).
std::vector<EventRow> eventsOf(const drogon::orm::DbClientPtr& db,
                               int64_t appId)
{
    std::promise<std::vector<EventRow>> p;
    auto f = p.get_future();
    *db << "SELECT id, application_id, from_status, to_status, comment,"
            " author_id, at FROM application_events WHERE application_id = $1"
            " ORDER BY id"
        << appId >>
        [&p](const drogon::orm::Result& rows) {
            std::vector<EventRow> v;
            for (const auto& r : rows)
            {
                EventRow e;
                e.id = r["id"].as<int64_t>();
                e.applicationId = r["application_id"].as<int64_t>();
                e.hasFrom = !r["from_status"].isNull();
                if (e.hasFrom)
                    e.from = r["from_status"].as<std::string>();
                e.to = r["to_status"].as<std::string>();
                e.hasComment = !r["comment"].isNull();
                if (e.hasComment)
                    e.comment = r["comment"].as<std::string>();
                e.authorId = r["author_id"].as<int64_t>();
                e.at = r["at"].as<std::string>();
                v.push_back(std::move(e));
            }
            p.set_value(std::move(v));
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(std::make_exception_ptr(
                std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

struct AppRow
{
    int64_t id = 0;
    std::string status;
    std::string updatedAt;
};

std::vector<AppRow> appsOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<std::vector<AppRow>> p;
    auto f = p.get_future();
    *db << "SELECT id, status, updated_at FROM applications ORDER BY id" >>
        [&p](const drogon::orm::Result& rows) {
            std::vector<AppRow> v;
            for (const auto& r : rows)
            {
                v.push_back({r["id"].as<int64_t>(),
                             r["status"].as<std::string>(),
                             r["updated_at"].as<std::string>()});
            }
            p.set_value(std::move(v));
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(std::make_exception_ptr(
                std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

int64_t eventCountOf(const drogon::orm::DbClientPtr& db)
{
    std::promise<int64_t> p;
    auto f = p.get_future();
    *db << "SELECT COUNT(*) AS n FROM application_events" >>
        [&p](const drogon::orm::Result& rows) {
            p.set_value(rows[0]["n"].as<int64_t>());
        } >>
        [&p](const DrogonDbException& e) {
            p.set_exception(
                std::make_exception_ptr(std::runtime_error(e.base().what())));
        };
    REQUIRE(f.wait_for(std::chrono::seconds(10)) == std::future_status::ready);
    return f.get();
}

bool statusIn(std::string_view s)
{
    for (const auto& st : kApplicationStatuses)
        if (st == s)
            return true;
    return false;
}

std::string statusAt(std::size_t i)
{
    return std::string(kApplicationStatuses[i]);
}

// id заявки, засеянной под статус словаря (см. Fixture::seedApplications).
int64_t appIdForStatus(std::size_t i)
{
    return static_cast<int64_t>(i + 1);
}

// id заявки, засеянной под пару матрицы.
int64_t appIdForPair(std::size_t i)
{
    return static_cast<int64_t>(11 + i);
}

}  // namespace

TEST_CASE("AppWorkflow: все 16 переходов матрицы применяются и пишут событие")
{
    Fixture f;
    REQUIRE(kApplicationStatusTransitions.size() == 16);

    for (std::size_t i = 0; i < kApplicationStatusTransitions.size(); ++i)
    {
        const auto& t = kApplicationStatusTransitions[i];
        const auto r = applyOf(f.db, appIdForPair(i), std::string(t.to),
                               std::nullopt, 5);
        CHECK(r.outcome == AppWorkflow::Outcome::Applied);
        CHECK(r.fromStatus == std::string(t.from));
    }

    // БД: каждая заявка пары теперь в целевом статусе; события — ровно по
    // одному на пару, поля дословно.
    const auto apps = appsOf(f.db);
    for (std::size_t i = 0; i < kApplicationStatusTransitions.size(); ++i)
    {
        const auto& t = kApplicationStatusTransitions[i];
        bool found = false;
        for (const auto& a : apps)
        {
            if (a.id != appIdForPair(i))
                continue;
            found = true;
            CHECK(a.status == std::string(t.to));
            // updated_at продлён UPDATE'ом (было 2026-01-01, DDL-формат).
            CHECK(a.updatedAt != "2026-01-01T00:00:00Z");
            CHECK(a.updatedAt.size() == 20);  // 2026-11-06T12:00:00Z
        }
        CHECK(found);

        const auto ev = eventsOf(f.db, appIdForPair(i));
        REQUIRE(ev.size() == 1);
        CHECK(ev[0].applicationId == appIdForPair(i));
        CHECK(ev[0].hasFrom);
        CHECK(ev[0].from == std::string(t.from));
        CHECK(ev[0].to == std::string(t.to));
        // comment не передан → SQL NULL (DDL: колонка nullable).
        CHECK_FALSE(ev[0].hasComment);
        CHECK(ev[0].authorId == 5);
        CHECK(ev[0].at.size() == 20);  // `at` — DEFAULT DDL (strftime)
    }
}

TEST_CASE("AppWorkflow: С7.6 — жизненный цикл одной заявки до «выдана»")
{
    // 01 §3 С7 (Дина, «Согласование темы ВКР»): draft → submitted →
    // accepted → in_progress → need_info («укажите кафедру») → ответ
    // студента → in_progress («В работе») → ready → issued. Всё в таймлайне.
    Fixture f;
    const int64_t app = 1;  // заявка в статусе draft (засев словаря)

    // Подача (студент) — С7.4.
    CHECK(applyOf(f.db, app, "submitted", std::nullopt, 4).outcome ==
          AppWorkflow::Outcome::Applied);
    // Приём и работа (сотрудник).
    CHECK(applyOf(f.db, app, "accepted", std::nullopt, 5).outcome ==
          AppWorkflow::Outcome::Applied);
    CHECK(applyOf(f.db, app, "in_progress", std::nullopt, 5).outcome ==
          AppWorkflow::Outcome::Applied);
    // Вопрос деканата с комментарием — С7.5.
    CHECK(applyOf(f.db, app, "need_info", std::string("укажите кафедру"), 5)
              .outcome == AppWorkflow::Outcome::Applied);
    // Ответ студента в мини-диалоге → «В работе» — С7.6: need_info →
    // in_progress входит в матрицу.
    CHECK(applyOf(f.db,
                  app,
                  "in_progress",
                  std::string("кафедра — мировая экономика"),
                  4)
              .outcome == AppWorkflow::Outcome::Applied);
    // Готово и выдано — С7.7.
    CHECK(applyOf(f.db, app, "ready", std::nullopt, 5).outcome ==
          AppWorkflow::Outcome::Applied);
    CHECK(applyOf(f.db, app, "issued", std::nullopt, 5).outcome ==
          AppWorkflow::Outcome::Applied);

    // Финальный статус и таймлайн — 7 событий, порядок по записи.
    const auto apps = appsOf(f.db);
    bool found = false;
    for (const auto& a : apps)
        if (a.id == app)
        {
            found = true;
            CHECK(a.status == "issued");
        }
    CHECK(found);

    const auto ev = eventsOf(f.db, app);
    REQUIRE(ev.size() == 7);
    const std::string expectedTo[] = {"submitted", "accepted", "in_progress",
                                      "need_info", "in_progress", "ready",
                                      "issued"};
    const std::string expectedFrom[] = {"draft", "submitted", "accepted",
                                        "in_progress", "need_info",
                                        "in_progress", "ready"};
    for (std::size_t i = 0; i < ev.size(); ++i)
    {
        CHECK(ev[i].from == expectedFrom[i]);
        CHECK(ev[i].to == expectedTo[i]);
    }
    // Комментарии: вопрос деканата (С7.5) и ответ студента (С7.6).
    CHECK(ev[3].hasComment);
    CHECK(ev[3].comment == "укажите кафедру");
    CHECK(ev[3].authorId == 5);
    CHECK(ev[4].hasComment);
    CHECK(ev[4].comment == "кафедра — мировая экономика");
    CHECK(ev[4].authorId == 4);
    // Остальные события без комментария; авторы: студент — подача,
    // сотрудник — остальные.
    CHECK_FALSE(ev[0].hasComment);
    CHECK(ev[0].authorId == 4);
    CHECK(ev[1].authorId == 5);
    CHECK_FALSE(ev[6].hasComment);
    CHECK(ev[6].authorId == 5);
}

TEST_CASE("AppWorkflow: все невалидные пары словаря → InvalidTransition, БД не тронута")
{
    Fixture f;

    // Снапшот до перебора: 9+16 заявок в исходных статусах, событий нет.
    const auto before = appsOf(f.db);
    REQUIRE(eventCountOf(f.db) == 0);

    // Исчерпывающий перебор всех пар словаря: невалидные (вне матрицы) —
    // 81 − 16 = 65. Включая названные в постановке submitted→issued и
    // ready→in_progress и поток 2 из 02 §5.2 («issued → in_progress нет»).
    int invalidCount = 0;
    for (std::size_t fi = 0; fi < kApplicationStatuses.size(); ++fi)
    {
        const auto from = statusAt(fi);
        for (std::size_t ti = 0; ti < kApplicationStatuses.size(); ++ti)
        {
            const auto to = statusAt(ti);
            if (AppWorkflow::canTransition(from, to))
                continue;
            ++invalidCount;
            const auto r =
                applyOf(f.db, appIdForStatus(fi), to, std::nullopt, 5);
            CHECK(r.outcome == AppWorkflow::Outcome::InvalidTransition);
            CHECK(r.fromStatus == from);
        }
    }
    CHECK(invalidCount == 65);
    // Названные в постановке примеры — именно в этом множестве.
    CHECK_FALSE(AppWorkflow::canTransition("submitted", "issued"));
    CHECK_FALSE(AppWorkflow::canTransition("ready", "in_progress"));
    CHECK_FALSE(AppWorkflow::canTransition("issued", "in_progress"));

    // Ничего не изменилось: статусы как были, таймлайн пуст.
    CHECK(eventCountOf(f.db) == 0);
    const auto after = appsOf(f.db);
    REQUIRE(after.size() == before.size());
    for (std::size_t i = 0; i < after.size(); ++i)
    {
        CHECK(after[i].id == before[i].id);
        CHECK(after[i].status == before[i].status);
        CHECK(after[i].updatedAt == before[i].updatedAt);
    }

    // Неизвестный статус назначения — вне матрицы → InvalidTransition
    // (и без записи в БД).
    const auto bogus = applyOf(f.db, 1, "bogus_status", std::nullopt, 5);
    CHECK(bogus.outcome == AppWorkflow::Outcome::InvalidTransition);
    CHECK(eventCountOf(f.db) == 0);

    // Неизвестные статусы на уровне константы: пары с «не из словаря»
    // не допустимы ни в одну сторону.
    for (const auto& s : kApplicationStatuses)
    {
        CHECK_FALSE(AppWorkflow::canTransition("bogus", s));
        CHECK_FALSE(AppWorkflow::canTransition(s, "bogus"));
    }
    CHECK_FALSE(AppWorkflow::canTransition("bogus", "bogus"));
}

TEST_CASE("AppWorkflow: нет заявки → NotFound; ошибка БД → Error (fail-closed)")
{
    Fixture f;
    const auto missing = applyOf(f.db, 999999, "accepted", std::nullopt, 5);
    CHECK(missing.outcome == AppWorkflow::Outcome::NotFound);
    CHECK(missing.fromStatus.empty());
    CHECK(eventCountOf(f.db) == 0);

    // Схемы нет: ошибка БД — Error (контроллер → 500), не InvalidTransition.
    Fixture broken(/*withTables=*/false);
    const auto err = applyOf(broken.db, 1, "submitted", std::nullopt, 5);
    CHECK(err.outcome == AppWorkflow::Outcome::Error);
}

TEST_CASE("Матрица согласована со словарём статусов CHECK DDL (02 §4.1)")
{
    CHECK(kApplicationStatuses.size() == 9);
    CHECK(kApplicationStatusTransitions.size() == 16);

    // Каждая пара — из словаря; дубликатов нет.
    for (std::size_t i = 0; i < kApplicationStatusTransitions.size(); ++i)
    {
        const auto& t = kApplicationStatusTransitions[i];
        CHECK(statusIn(t.from));
        CHECK(statusIn(t.to));
        for (std::size_t j = i + 1; j < kApplicationStatusTransitions.size();
             ++j)
        {
            const auto& u = kApplicationStatusTransitions[j];
            // && внутри CHECK декомпозируется doctest'ом — через bool.
            const bool samePair = (t.from == u.from) && (t.to == u.to);
            CHECK_FALSE(samePair);
        }
    }

    // Число исходящих переходов по статусу: терминальные — ноль,
    // «единственный переход из ready» — ровно issued (05 §3 Э5).
    const std::size_t expectedOut[] = {2, 3, 3, 4, 3, 1, 0, 0, 0};
    for (std::size_t i = 0; i < kApplicationStatuses.size(); ++i)
    {
        const auto s = statusAt(i);
        std::size_t out = 0;
        for (const auto& t : kApplicationStatusTransitions)
            if (t.from == s)
                ++out;
        CHECK(out == expectedOut[i]);
        for (const auto& other : kApplicationStatuses)
        {
            if (s != "ready" || other == "issued")
                continue;
            CHECK_FALSE(AppWorkflow::canTransition(s, other));
        }
    }

    // Гайд BE §7: submitted → accepted → in_progress → need_info|ready →
    // issued; С7.6: need_info → in_progress обязателен.
    CHECK(AppWorkflow::canTransition("draft", "submitted"));
    CHECK(AppWorkflow::canTransition("submitted", "accepted"));
    CHECK(AppWorkflow::canTransition("accepted", "in_progress"));
    CHECK(AppWorkflow::canTransition("in_progress", "need_info"));
    CHECK(AppWorkflow::canTransition("in_progress", "ready"));
    CHECK(AppWorkflow::canTransition("need_info", "in_progress"));  // С7.6
    CHECK(AppWorkflow::canTransition("ready", "issued"));
    // Отклонение и отмена студентом — по 01 §4.4 (ветки «отклонена»,
    // «отменена студентом») и словарю CHECK DDL.
    CHECK(AppWorkflow::canTransition("draft", "cancelled"));
    CHECK(AppWorkflow::canTransition("submitted", "rejected"));
    CHECK(AppWorkflow::canTransition("need_info", "rejected"));
}
