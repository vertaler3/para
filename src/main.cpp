#include <drogon/drogon.h>

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

#include "filters/CsrfCheck.h"
#include "filters/SecurityHeaders.h"
#include "services/MigrationRunner.h"
#include "services/NotifyDispatcher.h"
#include "services/ReminderService.h"
#include "services/WsRegistry.h"
#include "utils/ApiError.h"

int main(int argc, char* argv[]) {
    //Load config file: --config <path> — его передаёт ENTRYPOINT Dockerfile
    //(02 §8.1); --seed — режим фикстур (гайд BE, Э1 шаг 4). Drogon argv сам
    //не разбирает, поэтому минимальный разбор здесь.
    //По умолчанию ./config.json — локальный запуск без Docker.
    std::string configPath = "./config.json";
    bool seed = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--seed") {
            seed = true;
        } else if (arg == "--config" && i + 1 < argc) {
            configPath = argv[i + 1];
            ++i;
        }
    }
    // Э8 шаг 6 (02 §6.5 «секреты… в переменных окружения сервера»): config
    // разбирается вручную, чтобы применить переопределения окружения ДО
    // передачи в фреймворк (loadConfigJson — HttpAppFramework.h:469, v1.9.14;
    // loadConfigFile — тот же путь, но без точки вмешательства). Сейчас
    // переопределяется ровно один ключ: PARA_DB_PATH (env.example) — путь БД.
    // Он меняется В ОБОИХ местах сразу: custom_config.db_path (миграционный
    // раннер ниже) и db_clients[*].filename (пул Drogon) — иначе раннер
    // мигрировал бы один файл, а пул открыл бы другой. Переменная не задана —
    // работают значения config.json (дев-дефолт, /opt/para/data/para.db).
    Json::Value root;
    try {
        std::ifstream in(configPath, std::ios::binary);
        if (!in)
            throw std::runtime_error("config-файл не открыт: " + configPath);
        Json::CharReaderBuilder builder;
        std::string errs;
        // config.json содержит //-комментарии — jsoncpp читает их по умолчанию.
        if (!Json::parseFromStream(builder, in, &root, &errs))
            throw std::runtime_error("config не разобран (" + configPath +
                                     "): " + errs);
        if (const char* envDb = std::getenv("PARA_DB_PATH");
            envDb != nullptr && *envDb != '\0') {
            root["custom_config"]["db_path"] = envDb;
            if (root["db_clients"].isArray())
                for (Json::ArrayIndex i = 0; i < root["db_clients"].size(); ++i)
                    if (root["db_clients"][i].isObject())
                        root["db_clients"][i]["filename"] = envDb;
            // Факт переопределения — без значения пути (правило шага 6 Э8:
            // в логах факты/имена ключей, не значения).
            LOG_INFO << "конфиг: путь БД переопределён из PARA_DB_PATH";
        }
    } catch (const std::exception& e) {
        // Fail fast без деградации: без разобранного конфига приложение
        // не поднимаем (тот же принцип, что у миграций ниже).
        LOG_ERROR << "конфиг не загружен, запуск отменён: " << e.what();
        return 1;
    }
    drogon::app().loadConfigJson(std::move(root));

    //Миграции — при старте и только при нём (02 §8.3), до поднятия роутов:
    //раннер синхронный (libsqlite3), работает до app().run(), когда пул
    //DbClient ещё не создан. Пути — из custom_config config.json.
    try {
        const auto& cfg = drogon::app().getCustomConfig();
        para::MigrationRunner::run(
            cfg.get("db_path", "/opt/para/data/para.db").asString(),
            cfg.get("migrations_dir", "sql/migrations").asString(),
            cfg.get("fixtures_path", "sql/fixtures.sql").asString(),
            seed);
    } catch (const std::exception& e) {
        //Fail fast без деградации: с неприменённой схемой приложение
        //не поднимаем (гайд BE, Э1 шаг 4) — внятный лог и выход.
        LOG_ERROR << "миграции не применены, запуск отменён: " << e.what();
        return 1;
    }

    // Точка интеграции WS-канала (Э6 шаг 3; 02 §3.2 поток 3): NotifyDispatcher
    // пушит событие {type,title,body,deep_link} живым соединениям через
    // реестр WsRegistry (WsCtrl регистрирует их по user_id сессии, удаляет
    // по закрытию). До run() — колбэк обязан быть установлен до первого notify.
    para::NotifyDispatcher::setPushFn(
        [](int64_t userId, const Json::Value& payload) {
            para::WsRegistry::send(userId, payload);
        });

    //Run HTTP framework, the method will block in the internal event loop.
    //LoginFilter не регистрируется явно: HttpFilter<T> в Drogon создаётся
    //и регистрируется автоматически по имени класса ("LoginFilter") —
    //на него ссылаются роуты AuthCtrl через METHOD_LIST.

    // Выдача CSRF-токена на страницах (02 §6.4, Э8 шаг 2): post-handling
    // advice — ЕДИНАЯ точка issuance (Set-Cookie csrf при расхождении с
    // сессией + <meta name="csrf-token"> в <head> каждой страницы).
    // Состояния нет: токен перевычисляется от сессии (CsrfCheck.h). Тот же
    // вызов делает security_tests — сервер в тесте конфигурационно
    // идентичен продовому.
    para::CsrfCheck::registerPageIssuanceAdvice();

    // Заголовки безопасности на ВСЕХ ответах (02 §6.4, Э8 шаг 3):
    // nosniff, X-Frame-Options: DENY, CSP (default-src 'self' + inline
    // ровно для фактически inline style-атрибутов, без unsafe-eval),
    // Referrer-Policy. Advice на СОЗДАНИЕ ответа — единственная точка
    // на процесс, контроллеры не трогаются (SecurityHeaders.h).
    para::SecurityHeaders::registerAdvice();

    // Неизвестные пути API — единый формат ошибок 02 §5.1 (находка
    // Э1-404-HTML «Неизвестные /v1-пути отвечают HTML-404 Drogon», закрыта
    // кодом в шаге 7 Э8): router-miss под /v1/* и /ws/* отвечает
    // {"error":{"code":"not_found",…}} 404. Regex-роут drogon
    // (registerHandlerViaRegex, HttpAppFramework.h v1.9.14) проверяется
    // ПОСЛЕ статических роутов — живой прогон шага 7: /v1/me со сессией
    // → 200 (статические приоритетны), /v1/no-such → JSON. Публично и
    // без CSRF: путь не существует — никакого действия защищать нечего,
    // 404 не раскрывает больше, чем HTML-заглушка. SSR-пути (вне /v1|/ws)
    // остаются на HTML-заглушке Drogon (заголовки — SecurityHeaders).
    // Тот же вызов делает security_tests — сервер теста конфигурационно
    // идентичен продовому.
    drogon::app().registerHandlerViaRegex(
        "^/(?:v1|ws)/.*",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
            cb(para::api::errorResponse("not_found",
                                        "Путь не найден",
                                        drogon::k404NotFound));
        },
        {drogon::Get, drogon::Post, drogon::Put, drogon::Delete});

    // Таймер Э6 (гайд BE §9; 02 §4 поток 4): раз в минуту — базовое
    // напоминание «за 1 день» (deadlines.reminded), персональные
    // deadline_reminders и утренний дайджест тихих часов (шаг 2 Э6).
    // Регистрируется ПОСЛЕ запуска приложения: registerBeginningAdvice
    // срабатывает сразу после старта главного event loop (заголовок
    // HttpAppFramework.h v1.9.14) — только тогда существует getLoop(),
    // в котором живёт runEvery. Идемпотентность тика — метки
    // reminded/sent_at в БД: перезапуск не дублирует напоминания.
    // Клиент берётся ПО ИМЕНИ «para» (как в LoginFilter): getDbClient()
    // без параметров ищет клиента с именем «default» и возвращает пустой
    // указатель — сегфолт первого тика (поймано E2E шага 3, gdb под qemu
    // недоступен — стек: getDbClient(name="default") = nullptr).
    drogon::app().registerBeginningAdvice([] {
        drogon::app().getLoop()->runEvery(60.0, [] {
            para::ReminderService::runMinuteTick(
                drogon::app().getDbClient("para"), {});
        });
    });

    drogon::app().run();
    return 0;
}
