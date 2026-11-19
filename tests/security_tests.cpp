// security_tests — ПОЛНЫЙ HTTP-вариант автотеста безопасности 02 §9.3
// (Э8 шаг 1; DoD №2: «проверка выполняется фактически, не декларируется» —
// вывод прикладывается к отчёту).
//
// КАК РАБОТАЕТ (./build/security_tests самодостаточен — без внешнего
// оркестра, и в контейнере, и на любой машине):
//   1) MigrationRunner::run (тот же код, что в main.cpp) применяет
//      sql/migrations + sql/fixtures.sql (режим --seed) во временную
//      SQLite — ПОЛНАЯ схема и фикстуры прод-типа;
//   2) в тестовом процессе поднимается НАСТОЯЩИЙ сервер Drogon:
//      drogon::app().run() в отдельном потоке на 127.0.0.1:<свободный
//      порт>; CMake линкует в security_tests весь список PARA_APP_SOURCES
//      (те же контроллеры/фильтры/сервисы, что в цели para, кроме main);
//   3) все проверки ходят по реальному HTTP (drogon::HttpClient,
//      127.0.0.1). Две ЖИВЫЕ cookie-сессии: POST /v1/auth/login, токен из
//      Set-Cookie (SessionService::sessionCookie — флаги проверяются),
//      в каждом следующем запросе cookie возвращается, как в браузере.
//
// Статус по шагам 02 §9.3 (честно — имитация успеха запрещена, AGENTS.md).
// Каждый шаг проверяется фактически:
//   шаг 1 — Аня: POST /v1/applications → 201, в теле id_A (+контроль:
//           GET /v1/applications/{id_A} её же сессией → 200);
//   шаг 2 — Олег: GET /v1/applications/{id_A} → 403 forbidden;
//   шаг 3 — Олег: GET /v1/applications/999999 → 404 not_found;
//   шаг 4 — Олег: GET /v1/grades/{id_grade_ани} → 403 forbidden
//           (буквальная пара §9.3 Аня/Олег; фикстуры v2 дают Ане пустую
//           успеваемость — предусловие С5.6, поэтому ТЕСТОВАЯ БД добавляет
//           Ане одну оценку, иначе «чужой id» не определён — решение в
//           журнале Э8); +контроль: свой → 200; несуществующий → 404;
//   шаг 5 — Олег: POST /v1/notifications/{id_ани}/read → 403 forbidden;
//           ФАКТ НЕИЗМЕННОСТИ: лента Ани (?unread=1) по-прежнему содержит
//           id — чужая попытка ничего не пометила; +контроль: свой → 204
//           и строка уходит из unread; несуществующий → 404;
//   шаг 6 — Аня (student): GET /v1/staff/applications → 403 forbidden;
//           +контроль: dean.demo (staff) → 200 — 403 именно RBAC, а не
//           сломанный роут;
//   шаг 7 — без cookie: /v1/me → 401 unauthorized (API), /schedule → 303
//           Location: /login (SSR, 02 §6.3), /ws → 401 БЕЗ апгрейда;
//           +контроль: с cookie /v1/me → 200;
//   шаг 8 — Аня: POST /v1/rooms/1/bookings → 201, id брони; Олег:
//           DELETE /v1/room-bookings/{id_брони_ани} → 403 forbidden;
//           ФАКТ НЕИЗМЕННОСТИ: «мои брони» Ани по-прежнему содержат строку
//           booked; +контроль: свой DELETE → 204 и строка уходит из
//           списка; несуществующий id → 404;
//   +CSRF double-submit (02 §6.4, шаг 2 Э8): страница выдаёт токен (cookie
//           csrf + <meta name="csrf-token"> в <head>); state-changing БЕЗ
//           токена / с неверным заголовком / с ПОДДЕЛАННОЙ cookie csrf →
//           403 error.code=csrf; с валидным токеном → проходит (201/204);
//           GET-чтение и POST /v1/auth/login — вне CsrfCheck (спека:
//           state-changing только POST/PUT/DELETE; вход до сессии —
//           решение журнала Э8, AGENTS §4 смок сохранён);
//   +429 лимитера входа (02 §6.4): 3 неверных пароля под одним логином →
//           4-я попытка → 429 too_many_attempts + Retry-After (штатный
//           LoginLimiter: блок 15 с — тест НЕ ждёт разблокировку);
//           +зеркало лимитера в users (Э8 шаг 7, находка Э1 «неполное
//           зеркало»): неудача реального аккаунта пишет failed_attempts,
//           успешный вход сбрасывает его и locked_until (опрос БД —
//           зеркальный UPDATE асинхронный); для несуществующего логина
//           строки в users нет (UPDATE-only — ограничение зеркала,
//           зафиксировано в UserService.h);
//           +контроль изоляции: другой логин тем же паролем → 200;
//   +Формат 404 неизвестных путей (02 §5.1, Э8 шаг 7, находка Э1-404-HTML):
//           /v1/no-such → JSON {"error":{"code":"not_found"}}; /no-such
//           (SSR-сторона) → HTML-заглушка Drogon; заголовки безопасности —
//           на обоих (в блоке заголовков ниже);
//   +Заголовки безопасности (02 §6.4, шаг 3 Э8): nosniff / X-Frame-Options:
//           DENY / CSP / Referrer-Policy — точные значения на каждом
//           классе ответов (SSR, статика, JSON-401 фильтра, 303,
//           JSON-404 и HTML-404 роутера, авторизованная страница); в CSP
//           нет unsafe-eval.
//
// СЕССИИ/ПАРОЛЬ (AGENTS.md §5.2): логины — anya.demo (id 1), oleg.demo
// (id 2), dean.demo (id 5) из фикстур. Демо-пароль жюри тесту НЕ известен:
// ЕДИНЫЙ ТЕСТОВЫЙ пароль генерируется в памяти процесса
// (std::random_device), argon2id-хэши трёх фикстурных аккаунтов выставляет
// сам тест через PasswordHasher::hash() поверх временной БД; в
// репозитории и логах его нет.
//
// CSRF (02 §6.4) — с шага 2 Э8 реализован: CsrfCheck (signed double-submit,
// cookie csrf детерминированно привязан к сессии, CsrfCheck.h) вызывается из
// LoginFilter на всех приватных POST/PUT/DELETE. ВСЕ state-changing запросы
// этого файла ходят с cookie csrf + заголовком X-CSRF-Token (как браузер из
// app.js); отдельный блок ниже проверяет нет/расхождение/подделку → 403.
//
// Заголовки безопасности — с шага 3 Э8 реализованы: SecurityHeaders
// (advice на СОЗДАНИЕ ответа, registerHttpResponseCreationAdvice — самая
// ранняя точка, вызывается во всех фабриках drogon v1.9.14) ставит на ВСЕ
// ответы nosniff; X-Frame-Options: DENY; CSP default-src 'self' +
// style-src 'self' 'unsafe-inline' (единственный inline в проекте — два
// style-атрибута --value:N% в grades/grades_discipline; inline <script>
// и on*/hx-on в views/ нет; 'unsafe-eval' НЕ ставится) + frame-ancestors
// 'none'; Referrer-Policy: strict-origin-when-cross-origin (02 §6.4
// задаёт default-src 'self' / nosniff / frame-ancestors 'none' — всё на
// месте). Регистрация в тесте — тот же вызов, что в main.cpp (сервер
// конфигурационно идентичен продовому); блок ниже проверяет заголовки на
// каждом классе ответов: SSR-страница, статика, JSON-401 фильтра, 303,
// 404-заглушка роутера, авторизованная страница.
//
// Service-level проверки шагов 4/8 (Э4 шаг 3, Э5а шаг 2) ЗАМЕНЕНЫ этим
// HTTP-вариантом (сильнее: та же логика под настоящими LoginFilter/
// StaffFilter и контроллерами, живые сессии вместо user_id-параметра).
// Fail-closed ветка guard'а (ошибка БД → Error → 500) остаётся покрытой
// в para-tests: tests/unit/ownership_guard_test.cpp, TEST_CASE «ошибка БД
// → Error, fail-closed (не Own)».
//
// FK (Э8 шаг 5, находка Э1): на КАЖДОМ соединении этого файла включён
// «PRAGMA foreign_keys=ON» — и в тест-конфиге пула (initialization_queries,
// дословно как в config.json), и в raw-соединении сидера (RawDb); сид-данные
// FK-согласованы. В MigrationRunner enforcement НЕ включается — Р-24
// (no-op внутри его транзакций; schema-фаза — с FK OFF по руководству
// SQLite), согласованность валидирует гейт foreign_key_check в run().
//
// Любой FAIL → exit 1; при полном успехе печатается ровно строка
// «шаги 1–8 ПРОЙДЕН».

#include <drogon/drogon.h>
#include <drogon/Cookie.h>
#include <json/json.h>
#include <sqlite3.h>
#include <trantor/net/EventLoopThread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "filters/CsrfCheck.h"
#include "filters/SecurityHeaders.h"
#include "utils/NotFoundPage.h"
#include "utils/StaffRouting.h"
#include "services/MigrationRunner.h"
#include "services/PasswordHasher.h"
#include "services/SessionService.h"
#include "utils/ApiError.h"

namespace fs = std::filesystem;

namespace {

int g_checks = 0;
int g_failed = 0;

void check(bool ok, const std::string& what)
{
    ++g_checks;
    if (ok)
    {
        std::printf("  PASS  %s\n", what.c_str());
        return;
    }
    ++g_failed;
    std::printf("  FAIL  %s\n", what.c_str());
}

// Блок шага: печатает заголовок и итог по числу проверок внутри блока.
struct StepScope
{
    int checksAtEntry;
    int failedAtEntry;

    explicit StepScope(const char* title)
        : checksAtEntry(g_checks), failedAtEntry(g_failed)
    {
        std::printf("\n%s\n", title);
    }
    ~StepScope()
    {
        const int c = g_checks - checksAtEntry;
        const int f = g_failed - failedAtEntry;
        std::printf("  Итог: %d/%d проверок%s\n", c - f, c,
                    f == 0 ? "" : "  <- ЕСТЬ FAIL");
    }
    StepScope(const StepScope&) = delete;
    StepScope& operator=(const StepScope&) = delete;
};

// Временная SQLite + её файлы (как раньше: fs::temp_directory_path()).
struct TempDir
{
    fs::path path;
    TempDir()
    {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch()
                               .count();
        path = fs::temp_directory_path() /
               ("para-security-" + std::to_string(stamp));
        fs::create_directories(path);
    }
    ~TempDir()
    {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;
};

// Сервер и HTTP-клиент -------------------------------------------------------

drogon::HttpClientPtr g_client;

// Живая сессия: токен para_session + токен CSRF (cookie csrf из Set-Cookie
// ответа входа — как у браузера после POST /v1/auth/login). anon — запросы
// без сессии (публичные вход/healthz и шаг 7 «без cookie»).
struct Login
{
    std::string token;
    std::string csrf;
    drogon::Cookie cookie;
};
const Login anon;

// Синхронный HTTP-запрос к встроенному серверу. Возвращает nullptr только
// при транспортной ошибке (детали печатает вызывающий expect*).
// csrfHeader/csrfCookie — зеркало токена double-submit (как браузер из
// app.js): заголовок X-CSRF-Token + cookie csrf; пустые — запрос без токена
// (вход, GET-чтение и отрицательные CSRF-кейсы).
drogon::HttpResponsePtr sendHttp(drogon::HttpMethod method,
                                 const std::string& path,
                                 const std::string& sessionToken,
                                 const Json::Value* jsonBody,
                                 const std::string& csrfHeader = "",
                                 const std::string& csrfCookie = "")
{
    if (!g_client)
        return nullptr;
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(method);
    req->setPath(path);
    if (!sessionToken.empty())
        req->addCookie(para::SessionService::kCookieName, sessionToken);
    if (!csrfCookie.empty())
        req->addCookie(para::CsrfCheck::kCookieName, csrfCookie);
    if (!csrfHeader.empty())
        req->addHeader(para::CsrfCheck::kHeaderName, csrfHeader);
    if (jsonBody != nullptr)
    {
        // Каноничный путь drogon для JSON-запроса (см. newHttpJsonRequest):
        // код контента задаёт сериализацию и заголовок content-type.
        req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        Json::StreamWriterBuilder b;
        b["indentation"] = "";
        req->setBody(Json::writeString(b, *jsonBody));
    }
    auto pair = g_client->sendRequest(req, 10.0);
    if (pair.first != drogon::ReqResult::Ok || pair.second == nullptr)
        return nullptr;
    return pair.second;
}

drogon::HttpResponsePtr get(const Login& s, const std::string& path)
{
    // GET — чтение: токен CSRF не носится (02 §6.4: state-changing только
    // POST/PUT/DELETE).
    return sendHttp(drogon::Get, path, s.token, nullptr);
}

drogon::HttpResponsePtr post(const Login& s,
                             const std::string& path,
                             const Json::Value& body)
{
    return sendHttp(drogon::Post, path, s.token, &body, s.csrf, s.csrf);
}

drogon::HttpResponsePtr del(const Login& s, const std::string& path)
{
    return sendHttp(drogon::Delete, path, s.token, nullptr, s.csrf, s.csrf);
}

// Заголовок ответа без чувствительности к регистру (drogon хранит ключи
// в нижнем регистре, но проверяем обе формы).
std::string headerOf(const drogon::HttpResponse* r, const char* name)
{
    if (!r)
        return {};
    std::string v = r->getHeader(name);
    if (v.empty())
    {
        std::string lower(name);
        for (char& c : lower)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (lower != name)
            v = r->getHeader(lower);
    }
    return v;
}

// Токен para_session и его флаги читаются из Cookie-объекта ответа:
// клиентский парсер drogon превращает set-cookie в cookies_ (не в headers_,
// HttpResponseImpl.cc:998), поэтому getHeader("Set-Cookie") пуст.
drogon::Cookie sessionCookieOf(const drogon::HttpResponsePtr& r)
{
    return r ? r->getCookie(para::SessionService::kCookieName)
             : drogon::Cookie();
}

// Единые expect-обёртки: статус обязателен, код ошибки — там, где формат
// тела зафиксирован 02 §5.1 ({"error":{"code":...}}).
void expectStatus(const drogon::HttpResponsePtr& r,
                  drogon::HttpStatusCode expected,
                  const std::string& what,
                  const std::string& requestDesc)
{
    if (r == nullptr)
    {
        check(false, what + " — транспортная ошибка запроса (" +
                         requestDesc + ")");
        return;
    }
    if (r->getStatusCode() != expected)
    {
        std::string detail = " — получен код " +
                             std::to_string(
                                 static_cast<int>(r->getStatusCode()));
        const auto body = r->getJsonObject();
        if (body != nullptr && body->isMember("error") &&
            (*body)["error"].isMember("message"))
            detail += " [error.message=" +
                      (*body)["error"]["message"].asString() + "]";
        check(false, what + detail + " (" + requestDesc + ")");
        return;
    }
    check(true, what);
}

void expectErrorCode(const drogon::HttpResponse* r,
                     const char* expectedCode,
                     const std::string& what)
{
    const auto body = r ? r->getJsonObject() : nullptr;
    const std::string got = body && body->isMember("error") &&
                                    (*body)["error"].isMember("code")
                                ? (*body)["error"]["code"].asString()
                                : std::string("<нет error.code>");
    check(r != nullptr && got == expectedCode,
          what + (got == expectedCode ? "" : " — в теле error.code=" + got));
}

// Рекурсивный поиск объекта с полем id (форма ответов ленты/списков
// не завязана на имена массивов).
const Json::Value* findObjWithId(const Json::Value& node, int64_t id)
{
    if (node.isObject())
    {
        if (node.isMember("id") && node["id"].isIntegral() &&
            node["id"].asInt64() == id)
            return &node;
        for (const auto& key : node.getMemberNames())
        {
            const auto* found = findObjWithId(node[key], id);
            if (found != nullptr)
                return found;
        }
    }
    else if (node.isArray())
    {
        for (const auto& item : node)
        {
            const auto* found = findObjWithId(item, id);
            if (found != nullptr)
                return found;
        }
    }
    return nullptr;
}

// Тестовый пароль/токены — только в памяти процесса (AGENTS.md §5.2).
std::string randomHex(std::size_t bytes)
{
    static std::random_device rd;
    std::string out;
    out.reserve(bytes * 2);
    char buf[3];
    for (std::size_t i = 0; i < bytes; ++i)
    {
        std::snprintf(buf, sizeof buf, "%02x", rd() & 0xFFu);
        out += buf;
    }
    return out;
}

// Свободный порт для слушателя: bind :0 → getsockname → отпустить.
unsigned short findFreePort()
{
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return 0;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    bool ok = ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    if (ok)
        ok = ::listen(fd, 1) == 0;
    socklen_t len = sizeof(addr);
    if (ok)
        ok = ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len) == 0;
    ::close(fd);
    return ok ? ntohs(addr.sin_port) : 0;
}

// --- Сид тестовых данных поверх миграций+фикстур (raw sqlite3, до старта
// сервера). Схема — та же, что в проде; данные — вымышленные (AGENTS §5.1).

struct Seed
{
    int64_t anyaId = 0;
    int64_t olegId = 0;
    int64_t gradeAnyaId = 0;    // id_grade_ани (шаг 4)
    int64_t notifAnyaUnread = 0;  // цель чужой попытки шага 5 (не меняется)
    int64_t notifAnyaOwn = 0;     // контроль шага 5: свой → 204
};

class RawDb
{
  public:
    explicit RawDb(const std::string& path)
    {
        ok_ = sqlite3_open_v2(path.c_str(),
                              &db_,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                              nullptr) == SQLITE_OK;
        if (ok_)
        {
            // FK на каждом создаваемом соединении (Э8 шаг 5, находка Э1):
            // сид-данные обязаны быть FK-согласованы так же, как прод-пул
            // (config.json initialization_queries; SQLite по умолчанию
            // foreign_keys=OFF на каждом соединении — включается явно).
            ok_ = sqlite3_exec(
                      db_, "PRAGMA foreign_keys=ON;", nullptr, nullptr,
                      nullptr) == SQLITE_OK;
        }
    }
    ~RawDb()
    {
        if (db_ != nullptr)
            sqlite3_close(db_);
    }
    RawDb(const RawDb&) = delete;
    RawDb& operator=(const RawDb&) = delete;

    bool ok() const { return ok_ && db_ != nullptr; }

    // DDL/UPDATE без параметров.
    bool exec(const char* sql)
    {
        char* err = nullptr;
        if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK)
        {
            std::printf("  FAIL  raw-sql: %s (%s)\n", sql,
                        err != nullptr ? err : "?");
            if (err != nullptr)
                sqlite3_free(err);
            return false;
        }
        return true;
    }

    // Подготовленный оператор с текстовыми параметрами; возвращает число
    // изменённых строк и (для INSERT) rowid вставленной строки.
    bool runBound(const char* sql,
                  const std::vector<std::string>& params,
                  int& changes,
                  int64_t& lastRowId)
    {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::printf("  FAIL  prepare: %s (%s)\n", sql,
                        sqlite3_errmsg(db_));
            return false;
        }
        for (std::size_t i = 0; i < params.size(); ++i)
            sqlite3_bind_text(stmt, static_cast<int>(i + 1),
                              params[i].c_str(),
                              static_cast<int>(params[i].size()),
                              SQLITE_TRANSIENT);
        const int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (rc != SQLITE_DONE)
        {
            std::printf("  FAIL  step: %s (%s)\n", sql, sqlite3_errmsg(db_));
            return false;
        }
        changes = sqlite3_changes(db_);
        lastRowId = sqlite3_last_insert_rowid(db_);
        return true;
    }

    bool scalarInt64(const char* sql, int64_t& out)
    {
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;
        const int rc = sqlite3_step(stmt);
        const bool got = rc == SQLITE_ROW;
        if (got)
            out = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return got;
    }

  private:
    sqlite3* db_ = nullptr;
    bool ok_ = false;
};

bool seedTestData(const std::string& dbPath, const std::string& password,
                  Seed& seed)
{
    RawDb db(dbPath);
    if (!db.ok())
    {
        std::printf("  FAIL  seed: sqlite3_open(%s)\n", dbPath.c_str());
        return false;
    }
    int changes = 0;
    int64_t rowId = 0;

    // 1) Хэши трёх фикстурных аккаунтов → argon2id тестового пароля
    //    (соль индивидуальна: отдельный вызов hash() на аккаунт).
    for (const char* login : {"anya.demo", "oleg.demo", "dean.demo"})
    {
        const std::string hash = para::PasswordHasher::hash(password);
        if (!db.runBound("UPDATE users SET password_hash = $1,"
                         " failed_attempts = 0 WHERE login = $2",
                         {hash, login},
                         changes,
                         rowId) ||
            changes != 1)
        {
            std::printf("  FAIL  seed: UPDATE пароля для %s (rows=%d)\n",
                        login,
                        changes);
            return false;
        }
    }

    // 2) Фикстурные id — буквальная пара §9.3 (Аня id 1, Олег id 2).
    if (!db.scalarInt64("SELECT id FROM users WHERE login = 'anya.demo'",
                        seed.anyaId) ||
        !db.scalarInt64("SELECT id FROM users WHERE login = 'oleg.demo'",
                        seed.olegId) ||
        seed.anyaId != 1 || seed.olegId != 2)
    {
        std::printf("  FAIL  seed: фикстурные id Ани/Олега не 1/2 (%lld/%lld)"
                    "\n",
                    static_cast<long long>(seed.anyaId),
                    static_cast<long long>(seed.olegId));
        return false;
    }

    // 3) Оценка Ани (шаг 4 §9.3 требует существующий id_grade_ани; у Ани в
    //    v2 успеваемость пустая — см. шапку файла). Дисциплина 1 — ME-101.
    if (!db.runBound("INSERT INTO grades(user_id, discipline_id,"
                     " control_type, component, score, max_score, attempt,"
                     " comment, graded_at) VALUES ($1, 1, 'exam',"
                     " 'модуль 1', 30, 30, 1, NULL,"
                     " '2026-10-05T12:00:00Z')",
                     {std::to_string(seed.anyaId)},
                     changes,
                     rowId))
        return false;
    seed.gradeAnyaId = rowId;

    // 4) Два непрочитанных уведомления Ани (шаг 5): первое — цель чужой
    //    попытки Олега, второе — контроль своего → 204.
    if (!db.runBound("INSERT INTO notifications(user_id, type, title, body,"
                     " deep_link, read_at, created_at) VALUES"
                     " ($1, 'schedule', 'Перенос пары (тест security)',"
                     " 'Вымышленное уведомление автотеста 02 §9.3',"
                     " '/notifications', NULL,"
                     " '2026-11-10T12:00:00Z')",
                     {std::to_string(seed.anyaId)},
                     changes,
                     rowId))
        return false;
    seed.notifAnyaUnread = rowId;
    if (!db.runBound("INSERT INTO notifications(user_id, type, title, body,"
                     " deep_link, read_at, created_at) VALUES"
                     " ($1, 'deadline', 'Дедлайн (тест security)',"
                     " 'Вымышленное уведомление автотеста 02 §9.3',"
                     " '/deadlines', NULL,"
                     " '2026-11-10T12:00:01Z')",
                     {std::to_string(seed.anyaId)},
                     changes,
                     rowId))
        return false;
    seed.notifAnyaOwn = rowId;
    return true;
}

// Конфиг встроенного сервера: слушатель на 127.0.0.1:<port>, клиент БД
// «para» (то же имя, что ищут контроллеры), WAL — как в проде (config.json).
bool writeTestConfig(const std::string& configPath,
                     const std::string& dbPath,
                     unsigned short port)
{
    Json::Value listener;
    listener["address"] = "127.0.0.1";
    listener["port"] = static_cast<Json::Value::Int>(port);
    listener["https"] = false;
    Json::Value log;
    log["use_spdlog"] = false;
    log["log_level"] = "WARN";
    Json::Value appCfg;
    appCfg["number_of_threads"] = 2;
    appCfg["enable_session"] = false;
    // document_root — абсолютный путь к корню репо (прод задаёт тот же
    // ключ, config.json:29 «./»): ctest стартует в build/, ручной запуск —
    // из корня, статика для проверок заголовков обязана отдаваться в обоих
    // случаях (шаг 3 Э8).
    appCfg["document_root"] = std::string(PARA_SOURCE_DIR);
    appCfg["log"] = log;
    Json::Value dbClient;
    dbClient["name"] = "para";
    dbClient["rdbms"] = "sqlite3";
    dbClient["filename"] = dbPath;
    dbClient["connection_number"] = 1;
    Json::Value initQueries(Json::arrayValue);
    initQueries.append("PRAGMA journal_mode=WAL;");
    initQueries.append("PRAGMA synchronous=NORMAL;");
    initQueries.append("PRAGMA busy_timeout=5000;");
    // FK=ON на каждом соединении пула — тот же initialization_queries,
    // что в проде (config.json; Э8 шаг 5, находка Э1). Drogon 1.9.14
    // исполняет их на каждом новом соединении (Sqlite3Connection.cc).
    initQueries.append("PRAGMA foreign_keys=ON;");
    dbClient["initialization_queries"] = initQueries;
    Json::Value root;
    root["listeners"].append(listener);
    root["app"] = appCfg;
    root["db_clients"].append(dbClient);

    Json::StreamWriterBuilder b;
    b["indentation"] = "  ";
    std::ofstream out(configPath, std::ios::binary | std::ios::trunc);
    if (!out)
        return false;
    out << Json::writeString(b, root);
    return static_cast<bool>(out);
}

// Порядок teardown (поймано «corrupted double-linked list» на первом
// прогоне: HttpClient освобождался ПОСЛЕ остановки обоих event loop'ов):
// клиент закрывается, пока оба цикла ещё живы — раньше остановки сервера.
struct ClientReset
{
    ClientReset() = default;
    ~ClientReset() { g_client.reset(); }
    ClientReset(const ClientReset&) = delete;
    ClientReset& operator=(const ClientReset&) = delete;
};

// Сервер: поток с app().run(); деструктор останавливает и join-ит —
// корректное завершение и на зелёном, и на проваленном пути.
struct ServerGuard
{
    std::thread thread;
    ServerGuard() : thread([] { drogon::app().run(); }) {}
    ~ServerGuard()
    {
        drogon::app().quit();
        if (thread.joinable())
            thread.join();
    }
    ServerGuard(const ServerGuard&) = delete;
    ServerGuard& operator=(const ServerGuard&) = delete;
};

// Живые сессии ---------------------------------------------------------------

Login loginAs(const std::string& login, const std::string& password,
              const std::string& label)
{
    Json::Value body;
    body["login"] = login;
    body["password"] = password;
    Login out;
    auto r = post(anon, "/v1/auth/login", body);
    expectStatus(r,
                 drogon::k200OK,
                 label + ": POST /v1/auth/login → 200 (вход по реальному HTTP; "
                         "вход до сессии — вне CsrfCheck, решение Э8 шага 2)",
                 "POST /v1/auth/login " + login);
    if (r != nullptr)
        out.cookie = sessionCookieOf(r);
    out.token = out.cookie.value();
    check(!out.token.empty(),
          label + ": Set-Cookie " + para::SessionService::kCookieName +
              " → живой токен сессии");
    // CSRF double-submit (02 §6.4): вход выдаёт и cookie csrf — с неё
    // начинается жизнь токена у браузера (app.js читает cookie).
    const drogon::Cookie csrf = r ? r->getCookie(para::CsrfCheck::kCookieName)
                                  : drogon::Cookie();
    out.csrf = csrf.value();
    check(out.csrf.size() == 64,
          label + ": Set-Cookie csrf → токен double-submit (64 hex = 256 бит)");
    const bool csrfFlags = !csrf.isHttpOnly() && csrf.isSecure() &&
                           csrf.sameSite() == drogon::Cookie::SameSite::kLax;
    check(csrfFlags,
          label + ": cookie csrf — без HttpOnly (читает app.js); Secure; "
                  "SameSite=Lax (CsrfCheck::cookieFor)");
    return out;
}

}  // namespace

int main()
{
    std::printf(
        "security_tests — ПОЛНЫЙ HTTP-вариант автотеста безопасности "
        "02 §9.3 (Э8 шаги 1–3; закрытие находок — шаг 7; DoD №2).\n");
    std::printf("Встроенный сервер Drogon на 127.0.0.1:<свободный порт>; "
                "две живые cookie-сессии; схема+фикстуры прод-типа.\n");
    std::printf("Пароль тестовых сессий генерируется в памяти процесса и "
                "в лог не печатается (AGENTS.md §5.2).\n\n");

    TempDir tmp;
    const std::string dbPath = (tmp.path / "para.db").string();
    const std::string testPassword = randomHex(24);
    const std::string wrongPassword = "неверно-" + randomHex(8);

    // 1) Миграции + фикстуры (--seed) — код main.cpp, та же схема прод.
    bool migrationsOk = false;
    try
    {
        para::MigrationRunner::run(dbPath,
                                   std::string(PARA_SOURCE_DIR) +
                                       "/sql/migrations",
                                   std::string(PARA_SOURCE_DIR) +
                                       "/sql/fixtures.sql",
                                   true);
        migrationsOk = true;
    }
    catch (const std::exception& e)
    {
        std::printf("  FAIL  миграции/фикстуры: %s\n", e.what());
    }
    check(migrationsOk,
          "предусловие: миграции + фикстуры применены во временную SQLite");
    if (!migrationsOk)
        return 1;

    // 2) Сид тестовых данных (пароли argon2id, оценка и уведомления Ани).
    Seed seed;
    if (!seedTestData(dbPath, testPassword, seed))
    {
        check(false,
              "предусловие: сид тестовых данных (пароли/оценка/уведомления)");
        return 1;
    }
    check(true,
          "предусловие: argon2id-пароли трёх фикстурных аккаунтов выставлены "
          "тестом; id_grade=" +
              std::to_string(seed.gradeAnyaId) + ", уведомления " +
              std::to_string(seed.notifAnyaUnread) + "/" +
              std::to_string(seed.notifAnyaOwn));

    // 3) Свободный порт, конфиг, запуск сервера в отдельном потоке.
    const unsigned short port = findFreePort();
    if (port == 0)
    {
        check(false, "предусловие: свободный порт найден (bind :0)");
        return 1;
    }
    const std::string base = "http://127.0.0.1:" + std::to_string(port);
    const std::string configPath = (tmp.path / "config.json").string();
    if (!writeTestConfig(configPath, dbPath, port))
    {
        check(false, "предусловие: конфиг встроенного сервера записан");
        return 1;
    }
    drogon::app().loadConfigFile(configPath);
    // Выдача CSRF-токена на страницах — тот же вызов, что в main.cpp:
    // сервер в тесте обязан быть конфигурационно идентичен продовому
    // (CsrfCheck::registerPageIssuanceAdvice — post-handling advice).
    para::CsrfCheck::registerPageIssuanceAdvice();
    // Заголовки безопасности на всех ответах (Э8 шаг 3) — тот же вызов,
    // что в main.cpp (SecurityHeaders::registerAdvice — advice на
    // создание ответа, самая ранняя точка).
    para::SecurityHeaders::registerAdvice();
    // Страница «не найдено» v3 для SSR-путей (API — JSON-404 ниже).
    para::registerNotFoundPage();
    para::registerStaffPageRedirects();
    // Неизвестные пути API — JSON 02 §5.1 (Э8 шаг 7, находка Э1-404-HTML) —
    // тот же вызов, что в main.cpp: regex-роут проверяется после
    // статических (проверка ниже: /v1/me сессией → 200, /v1/no-such →
    // JSON-404, /no-such → HTML-заглушка).
    drogon::app().registerHandlerViaRegex(
        "^/(?:v1|ws)/.*",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
            cb(para::api::errorResponse("not_found",
                                        "Путь не найден",
                                        drogon::k404NotFound));
        },
        {drogon::Get, drogon::Post, drogon::Put, drogon::Delete});
    ServerGuard server;  // app().run() в отдельном потоке

    // HTTP-клиент — на собственном event loop (не loop приложения).
    trantor::EventLoopThread clientLoop;
    clientLoop.run();
    g_client = drogon::HttpClient::newHttpClient(base, clientLoop.getLoop());
    ClientReset clientResetGuard;  // деструкторы: клиент → loop → сервер

    // 4) Готовность: /healthz → 200 (публичный, без cookie).
    bool ready = false;
    for (int i = 0; i < 200 && !ready; ++i)
    {
        auto r = get(anon, "/healthz");
        ready = r != nullptr && r->getStatusCode() == drogon::k200OK;
        if (!ready)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    check(ready, "предусловие: сервер поднялся — GET /healthz → 200 (" +
                     base + ")");
    if (!ready)
        return 1;

    // 5) Две живые cookie-сессии §9.3 (+ staff-контроль шага 6).
    Login anya, oleg, dean;
    {
        StepScope s("Предусловия: две живые cookie-сессии (anya.demo / "
                    "oleg.demo) + dean.demo");
        anya = loginAs("anya.demo", testPassword, "Аня (id 1, student)");
        oleg = loginAs("oleg.demo", testPassword, "Олег (id 2, student)");
        dean = loginAs("dean.demo", testPassword,
                       "dean.demo (id 5, staff — контроль шага 6)");
        check(anya.token != oleg.token && anya.token != dean.token &&
                  oleg.token != dean.token,
              "сессии независимы: три разных токена (не одна сессия)");
        const bool flags = anya.cookie.isHttpOnly() &&
                           anya.cookie.isSecure() &&
                           anya.cookie.sameSite() ==
                               drogon::Cookie::SameSite::kLax;
        check(flags,
              "cookie сессии несёт HttpOnly; Secure; SameSite=Lax "
              "(SessionService::sessionCookie, 02 §6.1)");
    }

    // ====================================================================
    // Шаг 1: Аня создаёт заявку → id_A (02 §9.3).
    int64_t idA = 0;
    {
        StepScope s("Шаг 1 (02 §9.3): Аня создаёт заявку → id_A");
        Json::Value body;
        body["type"] = "academic_certificate";
        body["submit"] = true;
        auto r = post(anya, "/v1/applications", body);
        expectStatus(r,
                     drogon::k201Created,
                     "шаг 1: POST /v1/applications (Аня) → 201",
                     "POST /v1/applications type=academic_certificate");
        if (r != nullptr)
        {
            const auto parsed = r->getJsonObject();
            idA = parsed != nullptr && parsed->isMember("id")
                      ? (*parsed)["id"].asInt64()
                      : 0;
        }
        check(idA > 0,
              "шаг 1: в теле 201 — id_A=" + std::to_string(idA) + " (> 0)");
        if (idA > 0)
        {
            auto own =
                get(anya, "/v1/applications/" + std::to_string(idA));
            expectStatus(own,
                         drogon::k200OK,
                         "контроль шага 1: GET /v1/applications/{id_A} своей "
                         "сессией → 200 (403 дальше — именно про ЧУЖОЕ)",
                         "GET /v1/applications/" + std::to_string(idA));
        }
    }

    // ====================================================================
    // Шаг 2: чужая заявка → 403.
    {
        StepScope s("Шаг 2 (02 §9.3): Олег GET чужой заявки → 403");
        auto r = get(oleg, "/v1/applications/" + std::to_string(idA));
        expectStatus(r,
                     drogon::k403Forbidden,
                     "шаг 2: GET /v1/applications/{id_A} сессией Олега → 403 "
                     "(OwnershipGuard: Forbidden)",
                     "GET чужой applications/" + std::to_string(idA));
        expectErrorCode(r.get(), "forbidden",
                        "тело 403 — error.code=forbidden (формат 02 §5.1)");
    }

    // ====================================================================
    // Шаг 3: несуществующая заявка → 404.
    {
        StepScope s("Шаг 3 (02 §9.3): Олег GET /v1/applications/999999 → 404");
        auto r = get(oleg, "/v1/applications/999999");
        expectStatus(r,
                     drogon::k404NotFound,
                     "шаг 3: GET /v1/applications/999999 (Олег) → 404 "
                     "(NotFound не маскируется под 403)",
                     "GET /v1/applications/999999");
        expectErrorCode(r.get(), "not_found",
                        "тело 404 — error.code=not_found (формат 02 §5.1)");
    }

    // ====================================================================
    // Шаг 4: чужая оценка → 403 (+свой → 200, несуществующий → 404).
    {
        StepScope s("Шаг 4 (02 §9.3): Олег GET /v1/grades/{id_ани} → 403");
        const std::string gradePath =
            "/v1/grades/" + std::to_string(seed.gradeAnyaId);
        auto r = get(oleg, gradePath);
        expectStatus(r,
                     drogon::k403Forbidden,
                     "шаг 4: GET /v1/grades/" +
                         std::to_string(seed.gradeAnyaId) +
                         " сессией Олега → 403 (пару §9.3 вернули к "
                         "буквальной Аня/Олег — оценка добавлена тестом)",
                     "GET чужой " + gradePath);
        expectErrorCode(r.get(), "forbidden",
                        "тело 403 — error.code=forbidden");
        auto own = get(anya, gradePath);
        expectStatus(own,
                     drogon::k200OK,
                     "контроль шага 4: GET той же оценки сессией Ани → 200 "
                     "(свой объект доступен)",
                     "GET свой " + gradePath);
        auto missing = get(oleg, "/v1/grades/999999");
        expectStatus(missing,
                     drogon::k404NotFound,
                     "несуществующий grade (Олег → 999999) → 404",
                     "GET /v1/grades/999999");
    }

    // ====================================================================
    // Шаг 5: чужое уведомление → 403 + факт неизменности.
    {
        StepScope s("Шаг 5 (02 §9.3): Олег POST чужого read → 403");
        const std::string foreignPath =
            "/v1/notifications/" + std::to_string(seed.notifAnyaUnread) +
            "/read";
        auto r = post(oleg, foreignPath, Json::Value(Json::objectValue));
        expectStatus(r,
                     drogon::k403Forbidden,
                     "шаг 5: POST /v1/notifications/{id_ани}/read сессией "
                     "Олега → 403 (guard ДО пометки)",
                     "POST чужого read id=" +
                         std::to_string(seed.notifAnyaUnread));
        expectErrorCode(r.get(), "forbidden",
                        "тело 403 — error.code=forbidden");
        const bool foreignBlocked = r != nullptr &&
                                    r->getStatusCode() == drogon::k403Forbidden;

        // Факт неизменности (только при заблокированной чужой попытке —
        // иначе проверка тривиально «проходит» на упавшем запросе).
        auto feed = get(anya, "/v1/notifications?unread=1");
        expectStatus(feed,
                     drogon::k200OK,
                     "лента Ани GET /v1/notifications?unread=1 → 200",
                     "GET /v1/notifications?unread=1");
        bool stillUnread = false;
        if (feed != nullptr)
        {
            const auto parsed = feed->getJsonObject();
            stillUnread = parsed != nullptr &&
                          findObjWithId(*parsed, seed.notifAnyaUnread) !=
                              nullptr;
        }
        check(foreignBlocked && stillUnread,
              "факт неизменности: уведомление id=" +
                  std::to_string(seed.notifAnyaUnread) +
                  " по-прежнему в unread Ани (чужая попытка не пометила)");

        // Контроль: своё → 204 и строка уходит из unread.
        auto own = post(anya,
                        "/v1/notifications/" +
                            std::to_string(seed.notifAnyaOwn) + "/read",
                        Json::Value(Json::objectValue));
        expectStatus(own,
                     drogon::k204NoContent,
                     "контроль шага 5: свой POST read (Аня) → 204",
                     "POST своего read id=" +
                         std::to_string(seed.notifAnyaOwn));
        const bool ownMarked = own != nullptr &&
                               own->getStatusCode() == drogon::k204NoContent;
        auto feed2 = get(anya, "/v1/notifications?unread=1");
        bool gone = true;
        if (feed2 != nullptr)
        {
            const auto parsed = feed2->getJsonObject();
            gone = parsed == nullptr ||
                   findObjWithId(*parsed, seed.notifAnyaOwn) == nullptr;
        }
        check(ownMarked && gone,
              "факт пометки: после своего 204 строка id=" +
                  std::to_string(seed.notifAnyaOwn) +
                  " ушла из unread Ани");

        auto missing = post(oleg,
                            "/v1/notifications/999999/read",
                            Json::Value(Json::objectValue));
        expectStatus(missing,
                     drogon::k404NotFound,
                     "несуществующее уведомление (Олег → 999999/read) → 404",
                     "POST /v1/notifications/999999/read");
    }

    // ====================================================================
    // Шаг 6: студент на staff-роуте → 403 (+staff → 200).
    {
        StepScope s("Шаг 6 (02 §9.3): Аня вызывает staff-эндпоинт → 403");
        auto r = get(anya, "/v1/staff/applications");
        expectStatus(r,
                     drogon::k403Forbidden,
                     "шаг 6: GET /v1/staff/applications сессией Ани "
                     "(student) → 403 (StaffFilter)",
                     "GET /v1/staff/applications student-сессией");
        expectErrorCode(r.get(), "forbidden",
                        "тело 403 — error.code=forbidden");
        auto staff = get(dean, "/v1/staff/applications");
        expectStatus(staff,
                     drogon::k200OK,
                     "контроль шага 6: тот же роут сессией dean.demo "
                     "(staff) → 200 — 403 именно RBAC, не сломанный роут",
                     "GET /v1/staff/applications staff-сессией");
    }

    // ====================================================================
    // Шаг 7: без cookie — 401 (API), 303 (SSR), /ws без апгрейда.
    {
        StepScope s("Шаг 7 (02 §9.3): без cookie → 401/303; /ws → 401 "
                    "без апгрейда");
        auto me = get(anon, "/v1/me");
        expectStatus(me,
                     drogon::k401Unauthorized,
                     "шаг 7: GET /v1/me БЕЗ cookie → 401 (LoginFilter, "
                     "API-плоскость)",
                     "GET /v1/me без cookie");
        expectErrorCode(me.get(), "unauthorized",
                        "тело 401 — error.code=unauthorized");
        auto page = get(anon, "/schedule");
        expectStatus(page,
                     drogon::k303SeeOther,
                     "шаг 7: GET /schedule БЕЗ cookie → 303 (SSR-страница)",
                     "GET /schedule без cookie");
        check(headerOf(page.get(), "Location") == "/login",
              "303 ведёт на /login (02 §6.3)");
        // /ws — ЗАПРОС HANDSHAKE (RFC 6455: Upgrade + Sec-WebSocket-Key)
        // БЕЗ cookie: handshake обязан быть отклонён 401 ДО апгрейда
        // (контракт гейта Э6). Без upgrade-заголовков drogon не матчит
        // WS-роут вовсе (404 роутера — не ответ приложения).
        auto wsReq = drogon::HttpRequest::newHttpRequest();
        wsReq->setMethod(drogon::Get);
        wsReq->setPath("/ws");
        wsReq->addHeader("Upgrade", "websocket");
        wsReq->addHeader("Connection", "Upgrade");
        wsReq->addHeader("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
        wsReq->addHeader("Sec-WebSocket-Version", "13");
        drogon::HttpResponsePtr ws;
        {
            // Отдельный клиент: после handshake-запроса соединение
            // keep-alive общего клиента может быть закрыто сервером
            // (ответ на upgrade не является 101) — не мешаем следующим
            // проверкам (транспортная ошибка поймана на прогоне).
            auto wsClient =
                drogon::HttpClient::newHttpClient(base, clientLoop.getLoop());
            auto pair = wsClient->sendRequest(wsReq, 10.0);
            if (pair.first == drogon::ReqResult::Ok)
                ws = pair.second;
        }
        expectStatus(ws,
                     drogon::k401Unauthorized,
                     "шаг 7: GET /ws БЕЗ cookie → 401 (без апгрейда, "
                     "ответ до WebSocket-handshake)",
                     "GET /ws без cookie и без Upgrade");
        expectErrorCode(ws.get(), "unauthorized",
                        "тело 401 /ws — error.code=unauthorized");
        auto withCookie = get(oleg, "/v1/me");
        expectStatus(withCookie,
                     drogon::k200OK,
                     "контроль шага 7: /v1/me С cookie Олега → 200 "
                     "(401 выше — именно отсутствие сессии)",
                     "GET /v1/me с cookie");
    }

    // ====================================================================
    // Шаг 8: чужая бронь → 403 + факт неизменности (+свой → 204).
    int64_t idB = 0;
    {
        StepScope s("Шаг 8 (02 §9.3): Олег DELETE чужой брони → 403 "
                    "(бронь — тоже личный объект)");
        Json::Value slot;
        slot["starts_at"] = "2026-11-13T10:00:00Z";
        slot["ends_at"] = "2026-11-13T11:00:00Z";
        auto created = post(anya, "/v1/rooms/1/bookings", slot);
        expectStatus(created,
                     drogon::k201Created,
                     "предусловие шага 8: POST /v1/rooms/1/bookings (Аня, "
                     "13.11 10:00–11:00Z) → 201",
                     "POST /v1/rooms/1/bookings");
        if (created != nullptr)
        {
            const auto parsed = created->getJsonObject();
            idB = parsed != nullptr && parsed->isMember("id")
                      ? (*parsed)["id"].asInt64()
                      : 0;
        }
        check(idB > 0,
              "предусловие шага 8: в теле 201 — id брони=" +
                  std::to_string(idB));
        if (idB > 0)
        {

        auto r = del(oleg, "/v1/room-bookings/" + std::to_string(idB));
        expectStatus(r,
                     drogon::k403Forbidden,
                     "шаг 8: DELETE /v1/room-bookings/{id_ани} сессией "
                     "Олега → 403 (OwnershipGuard::RoomBookings)",
                     "DELETE чужой брони id=" + std::to_string(idB));
        expectErrorCode(r.get(), "forbidden",
                        "тело 403 — error.code=forbidden");
        const bool foreignBlocked = r != nullptr &&
                                    r->getStatusCode() == drogon::k403Forbidden;

        // Факт неизменности (только при заблокированной чужой попытке).
        auto list = get(anya, "/v1/room-bookings");
        expectStatus(list,
                     drogon::k200OK,
                     "GET /v1/room-bookings (Аня) → 200",
                     "GET /v1/room-bookings");
        const auto* row = list != nullptr && list->getJsonObject() != nullptr
                              ? findObjWithId(*list->getJsonObject(), idB)
                              : nullptr;
        check(foreignBlocked && idB > 0 && row != nullptr &&
                  row->isMember("status") && (*row)["status"] == "booked",
              "факт неизменности: бронь id=" + std::to_string(idB) +
                  " после чужого DELETE по-прежнему booked у Ани");

        // Контроль: своя бронь → 204 и уходит из списка активных.
        auto own = del(anya, "/v1/room-bookings/" + std::to_string(idB));
        expectStatus(own,
                     drogon::k204NoContent,
                     "контроль шага 8: свой DELETE (Аня) → 204",
                     "DELETE своей брони id=" + std::to_string(idB));
        const bool ownCancelled = own != nullptr &&
                                  own->getStatusCode() == drogon::k204NoContent;
        auto list2 = get(anya, "/v1/room-bookings");
        const auto* gone = list2 != nullptr &&
                                   list2->getJsonObject() != nullptr
                               ? findObjWithId(*list2->getJsonObject(), idB)
                               : nullptr;
        check(ownCancelled && idB > 0 && gone == nullptr,
              "факт отмены: бронь id=" + std::to_string(idB) +
                  " ушла из «моих броней» Ани (status='booked')");

        auto missing = del(oleg, "/v1/room-bookings/999999");
        expectStatus(missing,
                     drogon::k404NotFound,
                     "несуществующая бронь (Олег → 999999) → 404",
                     "DELETE /v1/room-bookings/999999");
        }
    }

    // ====================================================================
    // CSRF double-submit (02 §6.4, Э8 шаг 2): токен обязателен на всех
    // state-changing; нет/расхождение/подделка → 403 csrf; выдача токена
    // страницей; GET и вход вне проверки (спека + решение журнала Э8).
    {
        StepScope s("CSRF double-submit (02 §6.4, Э8 шаг 2): нет/расхождение "
                    "токена на state-changing → 403");
        const std::string appPath = "/v1/applications";
        Json::Value draft;
        draft["type"] = "academic_certificate";
        draft["submit"] = false;  // черновик — только проверка пропуска токена

        // 1) Выдача: страница несёт <meta name="csrf-token"> == cookie csrf
        //    (advice registerPageIssuanceAdvice, как браузерный путь).
        auto page = get(anya, "/schedule");
        expectStatus(page,
                     drogon::k200OK,
                     "GET /schedule (Аня) → 200 — страница отдаётся",
                     "GET /schedule с cookie");
        std::string metaToken;
        if (page != nullptr)
        {
            const std::string body(page->body());
            const std::string marker = "name=\"csrf-token\" content=\"";
            const auto at = body.find(marker);
            if (at != std::string::npos)
                metaToken = body.substr(at + marker.size(), 64);
        }
        check(!metaToken.empty() && metaToken == anya.csrf,
              "страница выдаёт токен: <meta name=\"csrf-token\"> == cookie "
              "csrf (выдача double-submit)");

        // 2) БЕЗ токена → 403 (никаких исключений для приватных POST).
        auto noTok = sendHttp(drogon::Post, appPath, anya.token, &draft);
        expectStatus(noTok,
                     drogon::k403Forbidden,
                     "POST /v1/applications без csrf (cookie и заголовка нет) "
                     "→ 403",
                     "POST " + appPath + " без токена");
        expectErrorCode(noTok.get(), "csrf",
                        "тело 403 — error.code=csrf (формат 02 §5.1)");

        // 3) Неверное ЗЕРКАЛО (cookie верная, заголовок иной) → 403.
        const std::string wrong = randomHex(32);
        auto badHdr = sendHttp(drogon::Post, appPath, anya.token, &draft,
                               wrong, anya.csrf);
        expectStatus(badHdr,
                     drogon::k403Forbidden,
                     "POST с X-CSRF-Token != cookie csrf → 403 (расхождение "
                     "double-submit)",
                     "POST " + appPath + " неверный заголовок");
        expectErrorCode(badHdr.get(), "csrf",
                        "тело 403 — error.code=csrf");

        // 4) ПОДДЕЛАННАЯ cookie csrf (cookie == заголовок == мусор) → 403:
        //    signed double-submit — cookie привязан к HttpOnly-сессии,
        //    подкинуть верное значение извне нельзя.
        auto forged = sendHttp(drogon::Post, appPath, anya.token, &draft,
                               wrong, wrong);
        expectStatus(forged,
                     drogon::k403Forbidden,
                     "POST с подделанной cookie csrf (cookie=заголовок) → "
                     "403 (привязка токена к сессии)",
                     "POST " + appPath + " подделка cookie csrf");
        expectErrorCode(forged.get(), "csrf",
                        "тело 403 — error.code=csrf");

        // 5) С валидным токеном → проходит (201; контрпример к 2–4 — 403
        //    именно про токен, а не сломанный роут).
        auto withTok = post(anya, appPath, draft);
        expectStatus(withTok,
                     drogon::k201Created,
                     "тот же POST с cookie csrf + X-CSRF-Token → 201 (роут "
                     "жив, 403 выше — именно про токен)",
                     "POST " + appPath + " с валидным токеном");

        // 6) GET — чтение: токен не требуется (02 §6.4: state-changing
        //    только POST/PUT/DELETE).
        auto readNoTok = sendHttp(drogon::Get, "/v1/me", oleg.token, nullptr);
        expectStatus(readNoTok,
                     drogon::k200OK,
                     "GET /v1/me без токена csrf → 200 (GET — вне CsrfCheck)",
                     "GET /v1/me без токена");

        // 7) Вход без csrf → 200: POST /v1/auth/login до сессии — токена
        //    ещё не существует (решение журнала Э8; AGENTS §4 смок).
        Login oleg2 = loginAs("oleg.demo", testPassword,
                              "повторный вход Олега (токена csrf нет)");
        // 8) Приватный state-changing с валидным токеном + инвалидация:
        //    logout Олега (второй сессии) → 204 (канон 02 §5.2, Э8 шаг 7 —
        //    находка Э1-logout-200 закрыта кодом), затем /v1/me → 401.
        Json::Value empty;
        auto out = post(oleg2, "/v1/auth/logout", empty);
        expectStatus(out,
                     drogon::k204NoContent,
                     "POST /v1/auth/logout с валидным токеном (вторая "
                     "сессия Олега) → 204 (02 §5.2)",
                     "POST /v1/auth/logout");
        auto afterOut = get(oleg2, "/v1/me");
        expectStatus(afterOut,
                     drogon::k401Unauthorized,
                     "/v1/me той же сессией после logout → 401 (сессия "
                     "инвалидирована на сервере, 02 §6.1)",
                     "GET /v1/me после logout");
    }

    // ====================================================================
    // 429: штатный LoginLimiter — 3 неверных входа → блок (02 §6.4).
    {
        StepScope s("429 лимитера входа (02 §6.4): 3 неверных пароля → "
                    "429 + Retry-After");
        const std::string target = "limiter-check.demo";
        Json::Value bad;
        bad["login"] = target;
        bad["password"] = wrongPassword;
        for (int attempt = 1; attempt <= 3; ++attempt)
        {
            auto r = post(anon, "/v1/auth/login", bad);
            expectStatus(r,
                         drogon::k401Unauthorized,
                         "неверная попытка " + std::to_string(attempt) +
                             "/3 (" + target + ") → 401",
                         "POST /v1/auth/login неверный пароль");
            expectErrorCode(r.get(), "invalid_credentials",
                            "тело 401 — error.code=invalid_credentials "
                            "(без деталей логин/пароль)");
        }
        auto blocked = post(anon, "/v1/auth/login", bad);
        expectStatus(blocked,
                     drogon::k429TooManyRequests,
                     "4-я попытка (" + target + ") → 429 too_many_attempts "
                     "(LoginLimiter: 3 неудачи → блок 15 с)",
                     "POST /v1/auth/login после 3 неудач");
        expectErrorCode(blocked.get(), "too_many_attempts",
                        "тело 429 — error.code=too_many_attempts");
        check(!headerOf(blocked.get(), "Retry-After").empty(),
              "429 несёт заголовок Retry-After (AuthCtrl)");

        // Зеркало лимитера в users (Э8 шаг 7 — находка Э1 «неполное
        // зеркало»: раньше писался только failed_attempts, locked_until
        // не писал никто, успешный вход не сбрасывал). Зеркальный UPDATE —
        // асинхронный, поэтому значения читаются из БД опросом с дедлайном
        // (до 2 с) — не «ожидание конкретного тайминга», а ожидание факта
        // записи. dean.demo: одна неудача → failed_attempts=1 (блок ещё
        // не включён — locked_until NULL), затем успех → сброс.
        RawDb mirrorDb(dbPath);
        check(mirrorDb.ok(), "зеркало: raw-соединение к тестовой БД открыто");
        const auto pollMirror = [&](const char* sql,
                                    int64_t expected,
                                    const char* what) {
            for (int i = 0; i < 40; ++i)
            {
                int64_t v = -1;
                if (mirrorDb.scalarInt64(sql, v) && v == expected)
                {
                    check(true, what);
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            check(false, what);
        };

        // 1 неудача dean.demo (блокировки ещё нет: 1 < 3).
        Json::Value deanBad;
        deanBad["login"] = "dean.demo";
        deanBad["password"] = wrongPassword;
        auto deanFail = post(anon, "/v1/auth/login", deanBad);
        expectStatus(deanFail,
                     drogon::k401Unauthorized,
                     "неудача dean.demo (1/3) → 401 (для проверки зеркала)",
                     "POST /v1/auth/login неверный пароль");
        pollMirror("SELECT failed_attempts FROM users WHERE login = "
                   "'dean.demo'",
                   1,
                   "зеркало: неудача dean.demo пишет users.failed_attempts=1 "
                   "(UserService::mirrorLimiterState)");
        pollMirror("SELECT COUNT(*) FROM users WHERE login = 'dean.demo' "
                   "AND locked_until IS NOT NULL",
                   0,
                   "зеркало: без блокировки (1<3) locked_until остаётся NULL");

        // Цель-призрак (лимитер-тест выше): строки в users нет — зеркало
        // UPDATE-only, для несуществующего логина ничего не пишет
        // (ограничение зеркала, зафиксировано в UserService.h).
        pollMirror("SELECT COUNT(*) FROM users WHERE login = "
                   "'limiter-check.demo'",
                   0,
                   "зеркало: у логина-призрака (limiter-check.demo) строки "
                   "в users нет — UPDATE-only, честное ограничение");

        // Контроль изоляции лимитера: другой логин не затронут; успешный
        // вход dean.demo заодно проверяет СБРОС зеркала.
        auto other = post(anon,
                          "/v1/auth/login",
                          [&] {
                              Json::Value v;
                              v["login"] = "dean.demo";
                              v["password"] = testPassword;
                              return v;
                          }());
        expectStatus(other,
                     drogon::k200OK,
                     "контроль изоляции: другой логин (dean.demo) НЕ "
                     "заблокирован → 200",
                     "POST /v1/auth/login dean.demo");
        pollMirror("SELECT failed_attempts FROM users WHERE login = "
                   "'dean.demo'",
                   0,
                   "зеркало: успешный вход сбрасывает users.failed_attempts "
                   "→ 0 (UserService::resetLimiterMirror)");
        pollMirror("SELECT COUNT(*) FROM users WHERE login = 'dean.demo' "
                   "AND locked_until IS NOT NULL",
                   0,
                   "зеркало: после успеха locked_until NULL");
    }

    // ====================================================================
    // Заголовки безопасности (02 §6.4, Э8 шаг 3): nosniff, X-Frame-Options:
    // DENY, CSP, Referrer-Policy — общий advice на СОЗДАНИЕ ответа
    // (SecurityHeaders::registerAdvice, тот же вызов, что в main.cpp),
    // не по-роутерно. Проверяются все классы ответов: публичная SSR-
    // страница, статика, JSON-401 фильтра, 303-редирект, 404-заглушка
    // роутера, авторизованные страницы.
    {
        StepScope s("Заголовки безопасности (02 §6.4, Э8 шаг 3): nosniff/"
                    "DENY/CSP/Referrer-Policy на всех ответах");

        // Проверка заголовка с честным FAIL и на транспортном сбое
        // (headerOf от nullptr вернул бы пустую строку — путаное сообщение).
        const auto expectHeader =
            [&](const drogon::HttpResponsePtr& r,
                const char* name,
                const std::string& want,
                const std::string& what) {
                const std::string got = r ? headerOf(r.get(), name)
                                          : std::string("<нет ответа>");
                check(got == want,
                      what +
                          (got == want ? "" : " — получено \"" + got + "\""));
            };

        // 1) Публичная SSR-страница (аноним): полный набор, ТОЧНЫЕ значения.
        auto login = get(anon, "/login");
        expectStatus(login,
                     drogon::k200OK,
                     "GET /login (аноним) → 200 — страница отдаётся",
                     "GET /login");
        expectHeader(login,
                     "X-Content-Type-Options",
                     "nosniff",
                     "GET /login: X-Content-Type-Options: nosniff");
        expectHeader(login,
                     "X-Frame-Options",
                     "DENY",
                     "GET /login: X-Frame-Options: DENY (плюс к "
                     "frame-ancestors 'none' — для браузеров без CSP)");
        expectHeader(login,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "GET /login: CSP == default-src 'self'; style-src "
                     "'self' 'unsafe-inline'; frame-ancestors 'none'");
        check(login == nullptr ||
                  headerOf(login.get(), "Content-Security-Policy")
                      .find("unsafe-eval") == std::string::npos,
              "GET /login: в CSP нет unsafe-eval (htmx-атрибуты его не "
              "требуют; inline <script> и on*/hx-on в views/ нет)");
        expectHeader(login,
                     "Referrer-Policy",
                     para::SecurityHeaders::kReferrerPolicyValue,
                     "GET /login: Referrer-Policy: strict-origin-when-"
                     "cross-origin");
        check(login != nullptr &&
                  std::string(login->body()).find("<!DOCTYPE html") !=
                      std::string::npos,
              "GET /login: тело — HTML-документ (страница отдаётся телом)");

        // 2) Главная анонимом — контракт 02 §6.3 (303 /login); заголовки
        //    обязательны и на редиректе.
        auto home = get(anon, "/");
        expectStatus(home,
                     drogon::k303SeeOther,
                     "GET / (аноним) → 303 /login — контракт SSR 02 §6.3",
                     "GET / аноним");
        expectHeader(home,
                     "X-Content-Type-Options",
                     "nosniff",
                     "GET /: nosniff стоит и на 303-редиректе");

        // 3) Статический файл — StaticFileRouter → newFileResponse.
        auto css = get(anon, "/static/css/app.css");
        expectStatus(css,
                     drogon::k200OK,
                     "GET /static/css/app.css → 200 — статика отдаётся",
                     "GET /static/css/app.css");
        expectHeader(css,
                     "X-Content-Type-Options",
                     "nosniff",
                     "GET /static/css/app.css: nosniff (MIME-защита статики)");
        expectHeader(css,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "GET /static/css/app.css: CSP на статике (ВСЕ ответы)");

        // 4) JSON-401 от LoginFilter — ответ ФИЛЬТРА, не контроллера.
        auto unauth = get(anon, "/v1/me");
        expectStatus(unauth,
                     drogon::k401Unauthorized,
                     "GET /v1/me (аноним) → 401 — ответ фильтра",
                     "GET /v1/me аноним");
        expectHeader(unauth,
                     "X-Content-Type-Options",
                     "nosniff",
                     "401 фильтра: nosniff");
        expectHeader(unauth, "X-Frame-Options", "DENY", "401 фильтра: DENY");
        expectHeader(unauth,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "401 фильтра: CSP");
        expectHeader(unauth,
                     "Referrer-Policy",
                     para::SecurityHeaders::kReferrerPolicyValue,
                     "401 фильтра: Referrer-Policy");

        // 5) 303-редирект LoginFilter на приватной SSR-странице.
        auto redirect = get(anon, "/schedule");
        expectStatus(redirect,
                     drogon::k303SeeOther,
                     "GET /schedule (аноним) → 303 /login",
                     "GET /schedule аноним");
        expectHeader(redirect,
                     "X-Content-Type-Options",
                     "nosniff",
                     "303 /schedule: nosniff");
        expectHeader(redirect,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "303 /schedule: CSP");

        // 6) Неизвестные пути, два класса (Э8 шаг 7 — находка Э1-404-HTML
        //    закрыта кодом): под /v1 — JSON 02 §5.1 (registerHandlerViaRegex
        //    в main.cpp/тесте), вне /v1 — HTML-заглушка Drogon. Заголовки
        //    обязаны быть на обоих; JSON-проверка заодно доказывает, что
        //    regex-роут НЕ заслоняет статические: /v1/me выше → 401
        //    контролируемым путём, /v1/applications шага 2–3 → 403/404
        //    контроллеров.
        auto lostApi = get(anon, "/v1/no-such-" + randomHex(6));
        expectStatus(lostApi,
                     drogon::k404NotFound,
                     "GET /v1/no-such-<hex> → 404 — JSON-формат API",
                     "GET несуществующий /v1-путь");
        expectErrorCode(lostApi.get(),
                        "not_found",
                        "тело 404 /v1 — error.code=not_found (формат "
                        "02 §5.1, Э8 шаг 7)");
        expectHeader(lostApi,
                     "X-Content-Type-Options",
                     "nosniff",
                     "JSON-404 /v1: nosniff");
        auto lostPage = get(anon, "/no-such-" + randomHex(6));
        expectStatus(lostPage,
                     drogon::k404NotFound,
                     "GET /no-such-<hex> → 404 — HTML-страница «не найдено» (SSR-сторона)",
                     "GET несуществующий SSR-путь");
        expectHeader(lostPage,
                     "X-Content-Type-Options",
                     "nosniff",
                     "HTML-404 роутера: nosniff");
        // fe_rebuild v3: SSR-404 — страница «не найдено» дизайна v3 (NotFoundPage),
        // не заглушка Drogon; API-404 выше остаётся JSON.
        check(lostPage != nullptr &&
                  std::string(lostPage->getBody()).find("data-page=\"not-found\"") !=
                      std::string::npos,
              "GET /no-such-<hex> → страница «не найдено» v3 (data-page=not-found)");

        // 7) Авторизованные страницы (живая сессия Ани): главная и
        //    /schedule — заголовки на месте после входа.
        auto homeAuth = get(anya, "/");
        expectStatus(homeAuth,
                     drogon::k200OK,
                     "GET / (Аня) → 200 — главная отдаётся",
                     "GET / с cookie");
        expectHeader(homeAuth,
                     "X-Content-Type-Options",
                     "nosniff",
                     "GET / (Аня): nosniff");
        expectHeader(homeAuth,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "GET / (Аня): CSP");
        auto sched = get(anya, "/schedule");
        expectStatus(sched,
                     drogon::k200OK,
                     "GET /schedule (Аня) → 200 — страница отдаётся",
                     "GET /schedule с cookie");
        expectHeader(sched,
                     "X-Content-Type-Options",
                     "nosniff",
                     "GET /schedule (Аня): nosniff");
        expectHeader(sched,
                     "Content-Security-Policy",
                     para::SecurityHeaders::kCspValue,
                     "GET /schedule (Аня): CSP (пост-логин страница)");
    }

    // --- Вердикт ------------------------------------------------------------
    std::printf(
        "\nsecurity_tests: %d/%d проверок пройдено (реальный HTTP на "
        "%s, сервер в этом же процессе).\n",
        g_checks - g_failed,
        g_checks,
        base.c_str());
    if (g_failed != 0)
    {
        std::printf("security_tests: ПРОВАЛ — есть FAIL выше (exit 1).\n");
        return 1;  // ServerGuard остановит сервер
    }
    std::printf("шаги 1–8 ПРОЙДЕН\n");
    return 0;
}
