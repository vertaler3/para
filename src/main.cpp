#include <drogon/drogon.h>
#include <string>

#include "services/MigrationRunner.h"
#include "services/ReminderService.h"

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
    drogon::app().loadConfigFile(configPath);

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

    //Run HTTP framework, the method will block in the internal event loop.
    //LoginFilter не регистрируется явно: HttpFilter<T> в Drogon создаётся
    //и регистрируется автоматически по имени класса ("LoginFilter") —
    //на него ссылаются роуты AuthCtrl через METHOD_LIST.

    // Таймер Э6 (гайд BE §9; 02 §4 поток 4): раз в минуту — базовое
    // напоминание «за 1 день» (deadlines.reminded), персональные
    // deadline_reminders и утренний дайджест тихих часов (шаг 2 Э6).
    // Регистрируется ПОСЛЕ запуска приложения: registerBeginningAdvice
    // срабатывает сразу после старта главного event loop (заголовок
    // HttpAppFramework.h v1.9.14) — только тогда существует getLoop(),
    // в котором живёт runEvery. Идемпотентность тика — метки
    // reminded/sent_at в БД: перезапуск не дублирует напоминания.
    drogon::app().registerBeginningAdvice([] {
        drogon::app().getLoop()->runEvery(60.0, [] {
            para::ReminderService::runMinuteTick(
                drogon::app().getDbClient(), {});
        });
    });

    drogon::app().run();
    return 0;
}
