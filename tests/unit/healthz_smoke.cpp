// Смок-тест каркаса (шаг 2 Э1): константа пути /healthz из HealthCtrl.
// doctest v2.5.3 — vendored одиночный заголовок, MIT
// (карточка лицензии: docs/decisions.md Р-3).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.hpp>

#include <string>

#include "controllers/HealthCtrl.h"

TEST_CASE("HealthCtrl: маршрут /healthz зафиксирован константой")
{
    CHECK(std::string(kHealthzPath) == "/healthz");
    CHECK(std::string(kHealthzPath).front() == '/');
}
