// Юнит-тесты AssistantService (идея 5.19, Р-36; шаг 4 журнала
// docs/agents-log/2026-11-20-ai-ассистент.md).
//
// Что проверяется:
//   - вход: пусто/не строка/пробелы → Empty (400), > 700 кодпоинтов →
//     TooLong (400), ровно 700 кириллицей (1400 байт) — Ok;
//   - лимит 10/мин на пользователя: 11-я попытка → отказ с Retry-After,
//     окно скользит, пользователи независимы (429);
//   - настройки из .env: раскрытие кавычек/\n, URI модели YandexGPT;
//   - разбор ответа модели (не-200 / мусор / пустой content → nullopt);
//   - fail-closed: транспорт-ошибка, недостижимый URL реального
//     HttpClient, «не настроено» → Unavailable (502), история НЕ пишется;
//     БД без таблиц → Error (500);
//   - предфильтр «вне компетенции»: арифметика («5*3», «сколько будет
//     2+2», «синус 30») и переводы — canned-ответ ДО модели (LLM не
//     вызывается), пара пишется в историю; вопросы портала не задеваются;
//   - изоляция: контекст и промт — только данные своей сессии (чужие
//     дедлайн/заявка не попадают), история — WHERE user_id = сессия.
// БД — реальные миграции + фикстуры (MigrationRunner, как в проде) во
// временном каталоге; модель — инъецированный фейковый транспорт, сеть
// наружу тесты не трогают.

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <doctest.hpp>
#include <drogon/orm/DbClient.h>
#include <trantor/net/EventLoopThread.h>

#include "services/AssistantService.h"
#include "services/MigrationRunner.h"
#include "services/SessionService.h"

namespace fs = std::filesystem;
using para::AssistantService;
using para::SessionInfo;

namespace {

constexpr auto kWait = std::chrono::seconds(30);

struct TempDir
{
    fs::path path;
    TempDir()
    {
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("para-assistant-" +
                std::to_string(std::chrono::steady_clock::now()
                                   .time_since_epoch()
                                   .count()) +
                "-" + std::to_string(++counter));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

void exec(const std::string& dbPath, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open(dbPath.c_str(), &raw) == SQLITE_OK);
    char* err = nullptr;
    const int rc = sqlite3_exec(raw, sql.c_str(), nullptr, nullptr, &err);
    if (err != nullptr)
    {
        MESSAGE(err);
        sqlite3_free(err);
    }
    sqlite3_close(raw);
    REQUIRE(rc == SQLITE_OK);
}

int count(const std::string& dbPath, const std::string& sql)
{
    sqlite3* raw = nullptr;
    REQUIRE(sqlite3_open(dbPath.c_str(), &raw) == SQLITE_OK);
    sqlite3_stmt* st = nullptr;
    REQUIRE(sqlite3_prepare_v2(raw, sql.c_str(), -1, &st, nullptr) ==
            SQLITE_OK);
    int n = -1;
    if (sqlite3_step(st) == SQLITE_ROW)
        n = sqlite3_column_int(st, 0);
    sqlite3_finalize(st);
    sqlite3_close(raw);
    return n;
}

// Демо-БД как в проде: все миграции (включая 0009) + фикстуры. Сверху —
// маркеры «чужих» данных с датой в далёком будущем (не зависят от «сейчас»):
// личный дедлайн Марка и личный дедлайн Ани.
struct Fixture
{
    TempDir tmp;
    std::string dbPath{(tmp.path / "para.db").string()};
    drogon::orm::DbClientPtr db;

    explicit Fixture(bool withSchema = true)
    {
        if (withSchema)
        {
            const std::string root = PARA_SOURCE_DIR;
            const auto r = para::MigrationRunner::run(
                dbPath, root + "/sql/migrations", root + "/sql/fixtures.sql",
                true);
            REQUIRE(r.appliedMigrations >= 9);
            exec(dbPath,
                 "INSERT INTO deadlines(group_id, user_id, title, kind, "
                 "due_at) VALUES "
                 "(2, 3, 'МАРКЕР-ЧУЖОЙ-ДЕДЛАЙН-МАРКА', 'work', "
                 "'2099-01-10T10:00:00Z'),"
                 "(1, 1, 'МАРКЕР-СВОЙ-ДЕДЛАЙН-АНИ', 'work', "
                 "'2099-01-11T10:00:00Z');");
        }
        db = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + dbPath, 1, {"PRAGMA foreign_keys=ON;"});
    }
};

SessionInfo session(int64_t id, const std::string& login)
{
    SessionInfo s;
    s.userId = id;
    s.login = login;
    s.role = "student";
    return s;
}

AssistantService::Config config()
{
    AssistantService::Config c;
    c.url = "http://fake.invalid/v1/chat/completions";
    c.key = "test-key";
    c.model = "gpt://folder/yandexgpt-lite/latest";
    c.systemPrompt = "SYSTEM-PROMPT-TEST";
    return c;
}

// Фейковый транспорт: запоминает payload, отвечает reply (nullopt — сбой).
struct FakeLlm
{
    std::shared_ptr<Json::Value> seen = std::make_shared<Json::Value>();
    std::shared_ptr<int> calls = std::make_shared<int>(0);
    std::optional<std::string> reply;
    AssistantService::Transport transport() const
    {
        auto seenP = seen;
        auto callsP = calls;
        auto r = reply;
        return [seenP, callsP, r](const AssistantService::Config&,
                                  const Json::Value& payload,
                                  AssistantService::ReplyCb cb) {
            *seenP = payload;
            ++*callsP;
            cb(r);
        };
    }
};

AssistantService::ChatResult runChat(Fixture& f,
                                     const SessionInfo& s,
                                     const std::string& message,
                                     const AssistantService::Config& cfg,
                                     AssistantService::Transport transport)
{
    std::promise<AssistantService::ChatResult> p;
    auto fut = p.get_future();
    AssistantService::chat(f.db, s, message, cfg, std::move(transport),
                           [&p](AssistantService::ChatResult r) {
                               p.set_value(std::move(r));
                           });
    REQUIRE(fut.wait_for(kWait) == std::future_status::ready);
    return fut.get();
}

Json::Value runHistory(Fixture& f, int64_t userId, int limit, bool* ok)
{
    std::promise<std::pair<bool, Json::Value>> p;
    auto fut = p.get_future();
    AssistantService::history(f.db, userId, limit,
                              [&p](bool good, Json::Value m) {
                                  p.set_value({good, std::move(m)});
                              });
    REQUIRE(fut.wait_for(kWait) == std::future_status::ready);
    auto r = fut.get();
    if (ok != nullptr)
        *ok = r.first;
    return r.second;
}

std::string repeat(const std::string& unit, int n)
{
    std::string s;
    for (int i = 0; i < n; ++i)
        s += unit;
    return s;
}

}  // namespace

TEST_CASE("ассистент: валидация входа — пусто 400, > 700 кодпоинтов 400")
{
    std::string out;
    CHECK(AssistantService::validate(nullptr, out) ==
          AssistantService::Input::Empty);  // тело не JSON

    Json::Value body(Json::objectValue);
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::Empty);  // нет поля
    body["message"] = 42;
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::Empty);  // не строка
    body["message"] = "  \n\t ";
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::Empty);  // одни пробелы

    body["message"] = "  что у меня завтра?  ";
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::Ok);
    CHECK(out == "что у меня завтра?");

    // Кириллица — 2 байта на символ: 700 символов = 1400 байт — допустимо.
    body["message"] = repeat("я", 700);
    CHECK(AssistantService::utf8Length(body["message"].asString()) == 700);
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::Ok);
    body["message"] = repeat("я", 701);
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::TooLong);
    body["message"] = repeat("я", 800);
    CHECK(AssistantService::validate(&body, out) ==
          AssistantService::Input::TooLong);
    CHECK(out.empty());
}

TEST_CASE("ассистент: rate-limit 10/мин на пользователя → 429 + Retry-After")
{
    AssistantService::RateLimiter lim;
    const int64_t t0 = 1'000'000;
    for (int i = 0; i < AssistantService::RateLimiter::kMax; ++i)
        CHECK(lim.acquire(1, t0 + i) == 0);
    // 11-я в той же минуте — отказ; ждать до освобождения первого слота.
    const int64_t wait = lim.acquire(1, t0 + 10);
    CHECK(wait == 50);
    CHECK(lim.acquire(1, t0 + 11) > 0);  // 12-я — тоже отказ
    // Другой пользователь — свой счётчик.
    CHECK(lim.acquire(2, t0 + 11) == 0);
    // Окно скользит: через 60 с после первой — слот свободен.
    CHECK(lim.acquire(1, t0 + 60) == 0);
    CHECK(lim.acquire(1, t0 + 60) > 0);
}

TEST_CASE("ассистент: настройки из .env — кавычки, \\n и URI модели")
{
    CHECK(AssistantService::unquoteEnv("plain") == "plain");
    CHECK(AssistantService::unquoteEnv("\"a\\nb \\\"c\\\" \\\\d\"") ==
          "a\nb \"c\" \\d");
    CHECK(AssistantService::unquoteEnv("'raw \\n'") == "raw \\n");
    CHECK(AssistantService::unquoteEnv("\"") == "\"");

    CHECK(AssistantService::resolveModel("yandexgpt-lite", "test-folder") ==
          "gpt://test-folder/yandexgpt-lite/latest");
    CHECK(AssistantService::resolveModel("yandexgpt-lite/rc", "test-folder") ==
          "gpt://test-folder/yandexgpt-lite/rc");
    CHECK(AssistantService::resolveModel("gpt://x/yandexgpt/latest",
                                         "test-folder") ==
          "gpt://x/yandexgpt/latest");
    CHECK(AssistantService::resolveModel("yandexgpt-lite", "") ==
          "yandexgpt-lite");

    // Пустое окружение → не настроено (fail-closed 502 в chat).
    ::unsetenv("ASSISTANT_API_URL");
    ::unsetenv("ASSISTANT_API_KEY");
    CHECK_FALSE(AssistantService::configFromEnv().configured());
    CHECK(config().configured());
}

TEST_CASE("ассистент: системный промт — дефолт из кода, env переопределяет")
{
    // Инцидент 21.11: .env в gitignore, git-деплой не переносил промт на
    // сервер — там оставался старый «мягкий» текст. Теперь канонический
    // промт зашит в код (kDefaultSystemPrompt) и применяется, когда env
    // пуст/отсутствует; env-значение переопределяет.
    ::setenv("ASSISTANT_API_URL", "https://example/v1", 0);
    ::setenv("ASSISTANT_API_KEY", "k", 0);
    ::setenv("ASSISTANT_MODEL", "yandexgpt-lite", 0);
    ::unsetenv("ASSISTANT_SYSTEM_PROMPT");
    const auto def = AssistantService::configFromEnv();
    REQUIRE(def.configured());
    CHECK(def.systemPrompt.find("ПРОВЕРКА РЕШЕНИЙ") != std::string::npos);
    CHECK(def.systemPrompt.find("Никогда не говори «ваш расчёт верен»") !=
          std::string::npos);
    CHECK(def.systemPrompt.find("БЕЗ перевода часовых поясов") !=
          std::string::npos);
    CHECK(def.systemPrompt.find("НАВИГАЦИЯ") != std::string::npos);

    // Пустое значение env — тоже дефолт (не «не настроено»).
    ::setenv("ASSISTANT_SYSTEM_PROMPT", "\"\"", 1);
    CHECK(AssistantService::configFromEnv().systemPrompt ==
          def.systemPrompt);

    // Явное значение — переопределение (однострочный промт без кавычек).
    ::setenv("ASSISTANT_SYSTEM_PROMPT", "TEST-OVERRIDE", 1);
    CHECK(AssistantService::configFromEnv().systemPrompt == "TEST-OVERRIDE");
    ::unsetenv("ASSISTANT_SYSTEM_PROMPT");
    ::unsetenv("ASSISTANT_API_URL");
    ::unsetenv("ASSISTANT_API_KEY");
    ::unsetenv("ASSISTANT_MODEL");
}

TEST_CASE("ассистент: предфильтр — арифметика/переводы не ходят в модель")
{
    // Инцидент 21.11 («5*3»): промптовый запрет вероятностный, операторная
    // запись решалась моделью 3/3 запросов. Теперь — детерминированный
    // фильтр ДО модели. Намеренно узкий: вопросы портала не задевает.
    CHECK(AssistantService::isOutOfScopeRequest("5*3"));
    CHECK(AssistantService::isOutOfScopeRequest("2+2"));
    CHECK(AssistantService::isOutOfScopeRequest(" 17 * 23 "));
    CHECK(AssistantService::isOutOfScopeRequest("сколько будет 2+2?"));
    CHECK(AssistantService::isOutOfScopeRequest("Чему равен синус 30"));
    CHECK(AssistantService::isOutOfScopeRequest(
        "Переведи на английский: я люблю"));
    CHECK_FALSE(AssistantService::isOutOfScopeRequest("что у меня завтра?"));
    CHECK_FALSE(AssistantService::isOutOfScopeRequest("какие пары 25.11?"));
    CHECK_FALSE(AssistantService::isOutOfScopeRequest("какие у меня пары 25"));
    CHECK_FALSE(AssistantService::isOutOfScopeRequest("пара 25 ноября?"));
    CHECK_FALSE(
        AssistantService::isOutOfScopeRequest("проверь моё решение"));

    // Полный путь: canned-ответ, LLM НЕ вызывается, пара в истории.
    Fixture f;
    FakeLlm llm;
    llm.reply = "не должно быть использовано";
    const auto r =
        runChat(f, session(1, "anya.demo"), "5*3", config(), llm.transport());
    REQUIRE(r.status == AssistantService::Status::Ok);
    REQUIRE(*llm.calls == 0);
    CHECK(r.reply.find("в портале нет такой информации") !=
          std::string::npos);
    CHECK(r.reply.find("не решаю примеры") != std::string::npos);
    CHECK(count(f.dbPath, "SELECT COUNT(*) FROM assistant_messages WHERE "
                          "user_id = 1") == 2);

    // Контроль: обычный вопрос — прежний путь, модель вызвана (1 вызов).
    FakeLlm llm2;
    llm2.reply = "Завтра пар нет.";
    const auto r2 = runChat(f, session(1, "anya.demo"), "что у меня завтра?",
                            config(), llm2.transport());
    REQUIRE(r2.status == AssistantService::Status::Ok);
    CHECK(r2.reply == "Завтра пар нет.");
    REQUIRE(*llm2.calls == 1);
}

TEST_CASE("ассистент: разбор ответа модели")
{
    const std::string ok =
        R"({"choices":[{"message":{"role":"assistant","content":"  Привет!  "}}]})";
    CHECK(AssistantService::parseReply(200, ok).value_or("") == "Привет!");
    CHECK_FALSE(AssistantService::parseReply(403, ok).has_value());
    CHECK_FALSE(AssistantService::parseReply(200, "не json").has_value());
    CHECK_FALSE(AssistantService::parseReply(200, R"({"choices":[]})")
                    .has_value());
    CHECK_FALSE(
        AssistantService::parseReply(
            200, R"({"choices":[{"message":{"content":"   "}}]})")
            .has_value());
    CHECK_FALSE(AssistantService::parseReply(
                    200, R"({"error":{"message":"denied"}})")
                    .has_value());
}

TEST_CASE("ассистент: контекст и промт — только данные своей сессии")
{
    Fixture f;
    FakeLlm llm;
    llm.reply = "Завтра пар нет.";
    const auto r = runChat(f, session(1, "anya.demo"), "что у меня завтра?",
                           config(), llm.transport());
    REQUIRE(r.status == AssistantService::Status::Ok);
    CHECK(r.reply == "Завтра пар нет.");
    REQUIRE(*llm.calls == 1);

    const Json::Value& p = *llm.seen;
    CHECK(p["model"].asString() == "gpt://folder/yandexgpt-lite/latest");
    CHECK(p["max_tokens"].asInt() == AssistantService::kMaxTokens);
    const auto& msgs = p["messages"];
    REQUIRE(msgs.size() == 2);  // system(промт+контекст) + вопрос
    CHECK(msgs[0]["role"].asString() == "system");
    const std::string sys = msgs[0]["content"].asString();
    CHECK(sys.rfind("SYSTEM-PROMPT-TEST", 0) == 0);
    CHECK(sys.find("Расписание твоей группы на завтра") != std::string::npos);
    CHECK(sys.find("МАРКЕР-СВОЙ-ДЕДЛАЙН-АНИ") != std::string::npos);
    // Чужое: личный дедлайн Марка (группа 2) и заявка Дины «в работе».
    CHECK(sys.find("МАРКЕР-ЧУЖОЙ-ДЕДЛАЙН-МАРКА") == std::string::npos);
    CHECK(sys.find("Академическая справка") == std::string::npos);
    CHECK(msgs[1]["role"].asString() == "user");
    CHECK(msgs[1]["content"].asString() == "что у меня завтра?");

    // Тот же запрос сессией Марка — видит свой маркер и не видит Анин.
    FakeLlm llm2;
    llm2.reply = "ok";
    REQUIRE(runChat(f, session(3, "mark.demo"), "дедлайны?", config(),
                    llm2.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys2 = (*llm2.seen)["messages"][0]["content"].asString();
    CHECK(sys2.find("МАРКЕР-ЧУЖОЙ-ДЕДЛАЙН-МАРКА") != std::string::npos);
    CHECK(sys2.find("МАРКЕР-СВОЙ-ДЕДЛАЙН-АНИ") == std::string::npos);

    // Пара записана в историю владельца: вопрос + ответ.
    CHECK(count(f.dbPath, "SELECT COUNT(*) FROM assistant_messages WHERE "
                          "user_id = 1") == 2);
}

TEST_CASE("ассистент: история изолирована по user_id и идёт в промт (≤10)")
{
    Fixture f;
    // 12 реплик Ани и 3 реплики Олега напрямую в таблицу.
    std::string sql = "INSERT INTO assistant_messages(user_id, role, content) "
                      "VALUES ";
    for (int i = 1; i <= 12; ++i)
        sql += std::string(i > 1 ? "," : "") + "(1, '" +
               (i % 2 ? "user" : "assistant") + "', 'аня-" +
               std::to_string(i) + "')";
    sql += ",(2, 'user', 'олег-секрет-1'),(2, 'assistant', 'олег-2'),"
           "(2, 'user', 'олег-3')";
    exec(f.dbPath, sql);

    bool ok = false;
    const auto anya = runHistory(f, 1, AssistantService::kHistoryForClient, &ok);
    REQUIRE(ok);
    REQUIRE(anya.size() == 12);
    CHECK(anya[0]["content"].asString() == "аня-1");    // по возрастанию
    CHECK(anya[11]["content"].asString() == "аня-12");
    for (const auto& m : anya)
        CHECK(m["content"].asString().find("олег") == std::string::npos);

    const auto oleg = runHistory(f, 2, 20, &ok);
    REQUIRE(oleg.size() == 3);
    CHECK(oleg[0]["content"].asString() == "олег-секрет-1");

    const auto last5 = runHistory(f, 1, 5, &ok);
    REQUIRE(last5.size() == 5);
    CHECK(last5[0]["content"].asString() == "аня-8");

    // В промт уходят последние 10 СВОИХ реплик, без чужих.
    FakeLlm llm;
    llm.reply = "ответ";
    REQUIRE(runChat(f, session(1, "anya.demo"), "вопрос", config(),
                    llm.transport())
                .status == AssistantService::Status::Ok);
    const auto& msgs = (*llm.seen)["messages"];
    REQUIRE(msgs.size() == 1 + AssistantService::kHistoryForPrompt + 1);
    CHECK(msgs[1]["content"].asString() == "аня-3");
    CHECK(msgs[10]["content"].asString() == "аня-12");
    for (const auto& m : msgs)
        CHECK(m["content"].asString().find("олег") == std::string::npos);
}

TEST_CASE("ассистент: fail-closed — ошибка LLM → 502, история не пишется")
{
    Fixture f;
    const auto before =
        count(f.dbPath, "SELECT COUNT(*) FROM assistant_messages");

    SUBCASE("транспорт вернул ошибку")
    {
        FakeLlm llm;  // reply = nullopt
        const auto r = runChat(f, session(1, "anya.demo"), "привет",
                               config(), llm.transport());
        CHECK(r.status == AssistantService::Status::Unavailable);
        CHECK(r.reply.empty());
    }
    SUBCASE("не настроено — модель даже не вызывается")
    {
        FakeLlm llm;
        llm.reply = "не должно прийти";
        auto cfg = config();
        cfg.key.clear();
        const auto r = runChat(f, session(1, "anya.demo"), "привет", cfg,
                               llm.transport());
        CHECK(r.status == AssistantService::Status::Unavailable);
        CHECK(*llm.calls == 0);
    }
    SUBCASE("реальный HttpClient: фейковый недостижимый URL")
    {
        trantor::EventLoopThread loopThread;
        loopThread.run();
        auto cfg = config();
        cfg.url = "http://127.0.0.1:1/v1/chat/completions";
        const auto r =
            runChat(f, session(1, "anya.demo"), "привет", cfg,
                    AssistantService::httpTransport(loopThread.getLoop()));
        CHECK(r.status == AssistantService::Status::Unavailable);
    }
    SUBCASE("URL без схемы")
    {
        auto cfg = config();
        cfg.url = "llm.example/v1";
        const auto r = runChat(f, session(1, "anya.demo"), "привет", cfg,
                               AssistantService::httpTransport(nullptr));
        CHECK(r.status == AssistantService::Status::Unavailable);
    }
    CHECK(count(f.dbPath, "SELECT COUNT(*) FROM assistant_messages") ==
          before);
}

TEST_CASE("ассистент: ошибка БД → Error (500), модель не вызывается")
{
    Fixture f(false);  // пустая БД без таблиц
    FakeLlm llm;
    llm.reply = "не должно прийти";
    const auto r = runChat(f, session(1, "anya.demo"), "привет", config(),
                           llm.transport());
    CHECK(r.status == AssistantService::Status::Error);
    CHECK(*llm.calls == 0);
    bool ok = true;
    runHistory(f, 1, 20, &ok);
    CHECK_FALSE(ok);
}

TEST_CASE("ассистент: свои заявки в работе попадают в промт")
{
    Fixture f;
    FakeLlm llm;
    llm.reply = "ok";
    // Дина (id 4) — «Академическая справка» в статусе in_progress (фикстуры).
    REQUIRE(runChat(f, session(4, "dina.demo"), "что с заявкой?", config(),
                    llm.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys = (*llm.seen)["messages"][0]["content"].asString();
    CHECK(sys.find("Академическая справка — статус: В работе") !=
          std::string::npos);
    // Олег (id 2) — «подана» видна ему, справки Дины в его промте нет.
    FakeLlm llm2;
    llm2.reply = "ok";
    REQUIRE(runChat(f, session(2, "oleg.demo"), "что с заявкой?", config(),
                    llm2.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys2 = (*llm2.seen)["messages"][0]["content"].asString();
    CHECK(sys2.find("статус: Подана") != std::string::npos);
    CHECK(sys2.find("Академическая справка") == std::string::npos);
}

TEST_CASE("ассистент: сбой одного источника контекста → Error, модели не уходит")
{
    Fixture f;
    exec(f.dbPath, "DROP TABLE deadline_reminders; DROP TABLE deadlines;");
    FakeLlm llm;
    llm.reply = "не должно прийти";
    const auto r = runChat(f, session(1, "anya.demo"), "дедлайны?", config(),
                           llm.transport());
    CHECK(r.status == AssistantService::Status::Error);
    CHECK(*llm.calls == 0);
}

// Фикс 21.11 (журнал 2026-11-21-ассистент-фиксы.md): аудитория из вопроса
// подмешивается из карты (как GET /v1/map/rooms?q=…), расписание в контексте
// помечено «твоей группы» и чужих групп не содержит, время — без перевода
// поясов, как в UI портала.
TEST_CASE("ассистент: контекст — аудитория из карты и только своя группа")
{
    // Извлечение номера аудитории: формы «1207», «Б-204», «в-216» → в
    // каноническом виде; вопросы без аудитории — пусто.
    CHECK(AssistantService::roomQuery("Где аудитория 1207?") == "1207");
    CHECK(AssistantService::roomQuery("как пройти в Б-204?") == "Б-204");
    CHECK(AssistantService::roomQuery("Что в ауд. в-216 завтра?") == "В-216");
    // р..я в нижнем регистре (у = D1 83 → У = D0 A3) — регресс upperRu.
    CHECK(AssistantService::roomQuery("где аудитория у-204?") == "У-204");
    CHECK(AssistantService::roomQuery("когда пара в 3105?") == "3105");
    CHECK(AssistantService::roomQuery("Какие у меня дедлайны?").empty());
    CHECK(AssistantService::roomQuery("пары у группы 4-МЭО в пятницу")
              .empty());

    // Группа из вопроса: канонизация регистра; не группа — пусто.
    CHECK(AssistantService::mentionedGroup("пары у группы 4-МЭО в пятницу") ==
          "4-МЭО");
    CHECK(AssistantService::mentionedGroup("расписание 1-фмо на неделю") ==
          "1-ФМО");
    CHECK(AssistantService::mentionedGroup("что у меня завтра?").empty());
    CHECK(AssistantService::mentionedGroup("где аудитория 1207?").empty());

    // Дата из вопроса (today = 2026-11-21, суббота): явные даты, ДД.ММ,
    // будни от today; прошедшее ДД.ММ → следующий год; «завтра»/без даты
    // не распознаются (эти дни уже в контексте).
    CHECK(AssistantService::mentionedDate("что за пары 25 ноября",
                                          "2026-11-21") == "2026-11-25");
    CHECK(AssistantService::mentionedDate("пары 25.11", "2026-11-21") ==
          "2026-11-25");
    CHECK(AssistantService::mentionedDate("расписание на 25.11.2026",
                                          "2026-11-21") == "2026-11-25");
    CHECK(AssistantService::mentionedDate("что в среду?", "2026-11-21") ==
          "2026-11-25");
    CHECK(AssistantService::mentionedDate("пара 5 ноября", "2026-11-21") ==
          "2027-11-05");
    CHECK(AssistantService::mentionedDate("что у меня завтра?",
                                          "2026-11-21")
              .empty());
    CHECK(AssistantService::mentionedDate("какие дедлайны?", "2026-11-21")
              .empty());

    // Время без перевода поясов: 10:00Z — это «10:00» (не «13:00» МСК),
    // как в остальных экранах портала.
    CHECK(AssistantService::portalTime("2026-11-20T10:00:00Z") == "10:00");
    CHECK(AssistantService::portalDateTime("2026-11-30T21:00:00Z") ==
          "30.11 21:00");

    Fixture f;
    // Чужая пара-маркер: дисциплина-маркер + событие группы 2 (4-МЭО)
    // сегодня в 12:00 Z — в контексте Ани её быть не может, в контексте
    // Марка (группа 2) она есть в разделе «Расписание твоей группы».
    exec(f.dbPath,
         "INSERT INTO disciplines(code, title, control_type) VALUES"
         " ('M-ARKER', 'МАРКЕР-ЧУЖАЯ-ПАРА', 'credit');"
         "INSERT INTO schedule_events(group_id, discipline_id, teacher_id,"
         " room, kind, starts_at, ends_at, status) VALUES"
         " (2, (SELECT id FROM disciplines WHERE title='МАРКЕР-ЧУЖАЯ-ПАРА'),"
         " NULL, '3105', 'seminar',"
         " strftime('%Y-%m-%dT12:00:00Z','now'),"
         " strftime('%Y-%m-%dT13:30:00Z','now'), 'normal');");

    FakeLlm llm;
    llm.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"), "Где аудитория 1207?",
                    config(), llm.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys = (*llm.seen)["messages"][0]["content"].asString();
    // Аудитория из вопроса — данными карты (фикстуры: 1207 → Корпус Б,
    // этаж 2, аудитория; UPDATE rooms в fixtures.sql).
    CHECK(sys.find("1207 — Корпус Б") != std::string::npos);
    CHECK(sys.find("этаж 2") != std::string::npos);
    CHECK(sys.find("аудитория") != std::string::npos);
    // Расписание помечено «твоей группы», чужих групп в контексте нет.
    CHECK(sys.find("Расписание твоей группы на сегодня") != std::string::npos);
    CHECK(sys.find("расписание твоей группы") != std::string::npos);
    CHECK(sys.find("МАРКЕР-ЧУЖАЯ-ПАРА") == std::string::npos);
    CHECK(sys.find("без перевода поясов") != std::string::npos);

    // Марк (группа 2) свою пару-маркер видит — положительный контроль;
    // вопрос о СВОЕЙ группе предупреждения о чужой не включает.
    FakeLlm llm2;
    llm2.reply = "ok";
    REQUIRE(runChat(f, session(3, "mark.demo"), "что у меня сегодня 4-МЭО?",
                    config(), llm2.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys2 = (*llm2.seen)["messages"][0]["content"].asString();
    CHECK(sys2.find("МАРКЕР-ЧУЖАЯ-ПАРА") != std::string::npos);
    CHECK(sys2.find("Твоя группа — 4-МЭО") != std::string::npos);
    CHECK(sys2.find("это НЕ твоя группа") == std::string::npos);

    // Вопрос о ЧУЖОЙ группе → детерминированное предупреждение в контексте
    // (аудит 21.11: модель подменяла расписание пользователя).
    FakeLlm llm4;
    llm4.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"),
                    "пары у группы 4-МЭО в пятницу", config(),
                    llm4.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys4 = (*llm4.seen)["messages"][0]["content"].asString();
    CHECK(sys4.find("Твоя группа — 1-ФМО") != std::string::npos);
    CHECK(sys4.find("группа 4-МЭО — это НЕ твоя группа") !=
          std::string::npos);
    CHECK(sys4.find("НЕ подставляй") != std::string::npos);

    // «Проверь моё решение» → контекст требует наводящий вопрос без итога.
    FakeLlm llm5;
    llm5.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"),
                    "Проверь моё решение задачи: 6 − 4 = 2", config(),
                    llm5.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys5 = (*llm5.seen)["messages"][0]["content"].asString();
    CHECK(sys5.find("проверить его решение") != std::string::npos);
    CHECK(sys5.find("наводящий вопрос") != std::string::npos);

    // Дата в вопросе («что за пары 25 ноября») → секция этой даты с
    // догруженными парами своей группы (живой кейс 21.11: «нет информации»).
    exec(f.dbPath,
         "INSERT INTO disciplines(code, title, control_type) VALUES"
         " ('D-ATE', 'МАРКЕР-ДАТА-2511', 'credit');"
         "INSERT INTO schedule_events(group_id, discipline_id, teacher_id,"
         " room, kind, starts_at, ends_at, status) VALUES"
         " (1, (SELECT id FROM disciplines WHERE title='МАРКЕР-ДАТА-2511'),"
         " NULL, '3105', 'lecture', '2026-11-25T10:00:00Z',"
         " '2026-11-25T11:30:00Z', 'normal');");
    FakeLlm llm7;
    llm7.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"), "что за пары у меня 25 ноября",
                    config(), llm7.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys7 = (*llm7.seen)["messages"][0]["content"].asString();
    CHECK(sys7.find("Расписание твоей группы на 25 ноября 2026") !=
          std::string::npos);
    CHECK(sys7.find("МАРКЕР-ДАТА-2511") != std::string::npos);
    CHECK(sys7.find("10:00") != std::string::npos);  // время без сдвига
    // Вопрос без даты — секции дат и маркера нет; «завтра» секцию не
    // включает (уже есть основная).
    FakeLlm llm8;
    llm8.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"), "что у меня завтра?",
                    config(), llm8.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys8 = (*llm8.seen)["messages"][0]["content"].asString();
    CHECK(sys8.find("МАРКЕР-ДАТА-2511") == std::string::npos);
    CHECK(sys8.find("Расписание твоей группы на 25 ноября") ==
          std::string::npos);
    // «проверь расписание» — не решение, маркер не включается.
    FakeLlm llm6;
    llm6.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"), "проверь расписание",
                    config(), llm6.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys6 = (*llm6.seen)["messages"][0]["content"].asString();
    CHECK(sys6.find("проверить его решение") == std::string::npos);

    // Вопрос без аудитории — секции карты в контексте нет.
    FakeLlm llm3;
    llm3.reply = "ok";
    REQUIRE(runChat(f, session(1, "anya.demo"), "какие дедлайны?", config(),
                    llm3.transport())
                .status == AssistantService::Status::Ok);
    const std::string sys3 = (*llm3.seen)["messages"][0]["content"].asString();
    CHECK(sys3.find("из карты портала") == std::string::npos);
    CHECK(sys3.find("это НЕ твоя группа") == std::string::npos);
}
