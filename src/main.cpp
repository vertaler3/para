#include <drogon/drogon.h>
#include <string>

int main(int argc, char* argv[]) {
    //Load config file: --config <path> — его передаёт ENTRYPOINT Dockerfile
    //(02 §8.1); Drogon argv сам не разбирает, поэтому минимальный разбор здесь.
    //По умолчанию ./config.json — локальный запуск без Docker.
    std::string configPath = "./config.json";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--config") {
            configPath = argv[i + 1];
            break;
        }
    }
    drogon::app().loadConfigFile(configPath);
    //Run HTTP framework, the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
