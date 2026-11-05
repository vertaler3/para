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

# SQLite: системная libsqlite3 (02 §2.2, public domain) — прямое использование
# миграционным раннером до старта event loop (docs/decisions.md Р-5);
# dev-пакет есть в build-образе drogonframework/drogon:v1.9.14, в рантайм-слой
# уже входит libsqlite3-0 (Dockerfile, Р-4).
# Нюанс образа: find_package(Drogon) подкладывает drogon'овский FindSQLite3
# (не создаёт цель SQLite::SQLite3, вместо неё — SQLite3_lib), а на машинах
# без Drogon-пакета сработает системный FindSQLite3 с SQLite::SQLite3.
# Поддерживаем оба, фактическую цель проверяем после find_package.
find_package(SQLite3 REQUIRED)
if(TARGET SQLite3_lib)
    set(PARA_SQLITE3_TARGET SQLite3_lib)
else()
    set(PARA_SQLITE3_TARGET SQLite::SQLite3)
endif()

# libargon2 (phc-winner-argon2, CC0/Apache-2.0 — 02 §2.2, Р-7): пароли —
# только argon2id (02 §6.2); MD5/SHA256 из Drogon Utilities для паролей
# запрещены. Заголовки/библиотека — apt-пакет libargon2-dev (Dockerfile).
find_path(ARGON2_INCLUDE_DIR argon2.h REQUIRED)
find_library(ARGON2_LIBRARY NAMES argon2 REQUIRED)

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
               src/controllers/HealthCtrl.cpp
               src/controllers/AuthCtrl.cpp
               src/services/MigrationRunner.cpp
               src/services/OwnershipGuard.cpp
               src/services/PasswordHasher.cpp
               src/services/SessionService.cpp
               src/services/UserService.cpp
               src/filters/LoginFilter.cpp
               src/utils/LoginLimiter.cpp)

target_include_directories(para PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/src
                           ${CMAKE_CURRENT_BINARY_DIR}/generated
                           ${ARGON2_INCLUDE_DIR})
# CSP-шаблоны (views/*.csp → /login и т.д.): генерация drogon_ctl create
# view в бинарный каталог (в репозитории только .csp-исходники).
# drogon_create_views — функция из cmake-пакета Drogon (DrogonUtilities).
drogon_create_views(para ${CMAKE_CURRENT_SOURCE_DIR}/views
                        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(para PRIVATE
                      ${PARA_DROGON_TARGETS}
                      ${PARA_SQLITE3_TARGET}
                      ${ARGON2_LIBRARY})

# ##############################################################################
# para-tests — doctest (vendored одиночный заголовок, MIT — docs/decisions.md Р-3).
# Юнит-тесты: tests/unit/ (02 §9.1).
add_executable(para-tests
               tests/unit/healthz_smoke.cpp
               tests/unit/migration_runner_test.cpp
               tests/unit/auth_unit.cpp
               tests/unit/ownership_guard_test.cpp
               src/controllers/HealthCtrl.cpp
               src/services/MigrationRunner.cpp
               src/services/OwnershipGuard.cpp
               src/services/PasswordHasher.cpp
               src/utils/LoginLimiter.cpp)

target_include_directories(para-tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                                   ${CMAKE_CURRENT_SOURCE_DIR}/tests
                                   ${ARGON2_INCLUDE_DIR})
target_link_libraries(para-tests PRIVATE
                      ${PARA_DROGON_TARGETS}
                      ${PARA_SQLITE3_TARGET}
                      ${ARGON2_LIBRARY})

enable_testing()
add_test(NAME para-tests COMMAND para-tests)

# ##############################################################################
# security_tests — до Э8 это заглушка (компилируется и печатает план).
# Реальная реализация — 8 шагов 02 §9.3: 403/404/401, изоляция данных,
# staff-эндпоинты (гайд BE §10; DoD №2 — вывод теста прикладывается к отчёту).
add_executable(security_tests tests/security_tests.cpp)

target_include_directories(security_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME security_tests COMMAND security_tests)
