cmake_minimum_required(VERSION 3.20)
project(para CXX)

# «Пара» — цифровой деканат (концепт хакатона).
# Каркас Э1, шаги 1–2: цели para, para-tests, security_tests.
# Стиль унаследован от `drogon_ctl create project` (drogonframework/drogon:v1.9.14)
# — см. docs/decisions.md Р-2.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Drogon CONFIG REQUIRED)

# ##############################################################################
# Требование: рантайм-бинарник без libdrogon.so/libtrantor.so (docs/decisions.md Р-1).
# Факт (проверено 05.11.2026 в образе drogonframework/drogon:v1.9.14):
# в /usr/local/lib лежат только libdrogon.a и libtrantor.a, а экспортируемая
# цель Drogon::Drogon — STATIC IMPORTED (DrogonTargets.cmake:54); отдельной
# цели Drogon::DrogonStatic пакет не экспортирует. Поэтому:
#   1) если окружение экспортирует статические цели — линкуемся с ними;
#   2) иначе — Drogon::Drogon (в официальном образе это статическая цель);
#      факт проверяется ldd собранного бинарника (нет libdrogon.so — см. Р-1).
if(TARGET Drogon::DrogonStatic AND TARGET Trantor::TrantorStatic)
    set(PARA_DROGON_TARGETS Drogon::DrogonStatic Trantor::TrantorStatic)
    message(STATUS "para: linking Drogon/Trantor via explicit STATIC targets")
else()
    set(PARA_DROGON_TARGETS Drogon::Drogon)
    message(STATUS "para: linking Drogon::Drogon (STATIC IMPORTED in drogonframework/drogon:v1.9.14; verify runtime binary with ldd — no libdrogon.so)")
endif()

add_executable(para
               src/main.cpp
               src/controllers/HealthCtrl.cpp)

target_include_directories(para PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(para PRIVATE ${PARA_DROGON_TARGETS})

# ##############################################################################
# para-tests — doctest (vendored одиночный заголовок, MIT — docs/decisions.md Р-3).
# Юнит-тесты: tests/unit/ (02 §9.1).
add_executable(para-tests
               tests/unit/healthz_smoke.cpp
               src/controllers/HealthCtrl.cpp)

target_include_directories(para-tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                                   ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(para-tests PRIVATE ${PARA_DROGON_TARGETS})

enable_testing()
add_test(NAME para-tests COMMAND para-tests)

# ##############################################################################
# security_tests — до Э8 это заглушка (компилируется и печатает план).
# Реальная реализация — 8 шагов 02 §9.3: 403/404/401, изоляция данных,
# staff-эндпоинты (гайд BE §10; DoD №2 — вывод теста прикладывается к отчёту).
add_executable(security_tests tests/security_tests.cpp)

target_include_directories(security_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME security_tests COMMAND security_tests)
