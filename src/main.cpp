#include <drogon/drogon.h>

int main() {
    //Load config file (listener 0.0.0.0:8080, log; конфиг без секретов — AGENTS.md §5.2)
    drogon::app().loadConfigFile("./config.json");
    //Run HTTP framework, the method will block in the internal event loop
    drogon::app().run();
    return 0;
}
