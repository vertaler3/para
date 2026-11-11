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

# libharu (Э5 шаг 4, docs/decisions.md Р-12): PDF-справки со штампом «ДЕМО».
# ЛИЦЕНЗИЯ zlib/libPNG (пермиссивная, без копилефта) — проверена по LICENSE
# тега v2.4.6. Собирается ИЗ ИСХОДНИКОВ на build-слое Dockerfile
# (02 §8.1: пин коммита тега v2.4.6; пакета libhpdf-dev в noble нет).
# Локально/в контейнере — собранный префикс: -DHPDF_ROOT=/path/to/prefix
# (include/ + lib/libhpdf.a); в системе — /usr/local после cmake --install.
# Статическая библиотека тянет zlib (HPDF_USE... сжатие потоков).
set(HPDF_ROOT "" CACHE PATH "libharu install prefix (include/ + lib/)")
find_path(HPDF_INCLUDE_DIR hpdf.h HINTS ${HPDF_ROOT}/include /usr/local/include)
find_library(HPDF_LIBRARY NAMES hpdf HINTS ${HPDF_ROOT}/lib /usr/local/lib)
find_package(ZLIB REQUIRED)
if(NOT HPDF_INCLUDE_DIR OR NOT HPDF_LIBRARY)
    message(FATAL_ERROR
            "libharu не найдена: собери её на build-слое (Dockerfile Э5) или "
            "передай -DHPDF_ROOT=<prefix с include/hpdf.h и lib/libhpdf.a>")
endif()
message(STATUS "para: libharu — ${HPDF_LIBRARY} (${HPDF_INCLUDE_DIR})")

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

# ##############################################################################
# Исходники приложения БЕЗ main.cpp — общий список целей para и security_tests
# (Э8 шаг 1: security_tests поднимает НАСТОЯЩИЙ сервер в тестовом процессе
# и ходит по нему реальным HTTP — полный вариант автотеста 02 §9.3).
set(PARA_APP_SOURCES
               src/controllers/HealthCtrl.cpp
               src/controllers/AuthCtrl.cpp
               src/controllers/SettingsCtrl.cpp
               src/controllers/ScheduleCtrl.cpp
               src/controllers/GradesCtrl.cpp
               src/controllers/ApplicationsCtrl.cpp
               src/controllers/StaffCtrl.cpp
               src/controllers/PdfCtrl.cpp
               src/controllers/IcsCtrl.cpp
               src/controllers/RoomsCtrl.cpp
               src/controllers/WsCtrl.cpp
               src/controllers/AnnouncementsCtrl.cpp
               src/controllers/NotificationsFeedCtrl.cpp
               src/controllers/PeopleCtrl.cpp
               src/controllers/GradebookCtrl.cpp
               src/controllers/RatingCtrl.cpp
               src/controllers/LibraryCtrl.cpp
               src/controllers/CanteensCtrl.cpp
               src/controllers/MapCtrl.cpp
               src/controllers/DashboardCtrl.cpp
               src/controllers/ProfileCtrl.cpp
               src/services/MigrationRunner.cpp
               src/services/OwnershipGuard.cpp
               src/services/PasswordHasher.cpp
               src/services/SessionService.cpp
               src/services/SettingsService.cpp
               src/services/ScheduleService.cpp
               src/services/ScheduleCache.cpp
               src/services/GradesService.cpp
               src/services/GradebookService.cpp
               src/services/RatingService.cpp
               src/services/AppWorkflow.cpp
               src/services/ApplicationsService.cpp
               src/services/PdfBuilder.cpp
               src/services/IcsService.cpp
               src/services/IcsBuilder.cpp
               src/services/UserService.cpp
               src/services/RoomsService.cpp
               src/services/NotifyDispatcher.cpp
               src/services/ReminderService.cpp
               src/services/ScheduleChangeService.cpp
               src/services/StaffDashboard.cpp
               src/services/AnnouncementsService.cpp
               src/services/NotificationsFeedService.cpp
               src/services/WsRegistry.cpp
               src/services/PeopleService.cpp
               src/services/LibraryService.cpp
               src/services/CanteensService.cpp
               src/services/MapService.cpp
               src/services/DashboardService.cpp
               src/filters/LoginFilter.cpp
               src/filters/CsrfCheck.cpp
               src/filters/StaffFilter.cpp
               src/utils/LoginLimiter.cpp
               src/utils/NameMask.cpp)

add_executable(para
               src/main.cpp
               ${PARA_APP_SOURCES})

target_include_directories(para PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/src
                           ${CMAKE_CURRENT_BINARY_DIR}/generated
                           ${ARGON2_INCLUDE_DIR}
                           ${HPDF_INCLUDE_DIR})
# CSP-шаблоны (views/*.csp → /login и т.д.): генерация drogon_ctl create
# view в бинарный каталог (в репозитории только .csp-исходники).
# drogon_create_views — функция из cmake-пакета Drogon (DrogonUtilities).
drogon_create_views(para ${CMAKE_CURRENT_SOURCE_DIR}/views
                        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(para PRIVATE
                      ${PARA_DROGON_TARGETS}
                      ${PARA_SQLITE3_TARGET}
                      ${ARGON2_LIBRARY}
                      ${HPDF_LIBRARY}
                      ZLIB::ZLIB)

# ##############################################################################
# para-tests — doctest (vendored одиночный заголовок, MIT — docs/decisions.md Р-3).
# Юнит-тесты: tests/unit/ (02 §9.1).
add_executable(para-tests
               tests/unit/healthz_smoke.cpp
               tests/unit/migration_runner_test.cpp
               tests/unit/auth_unit.cpp
               tests/unit/ownership_guard_test.cpp
               tests/unit/settings_service_test.cpp
               tests/unit/schedule_service_test.cpp
               tests/unit/grades_service_test.cpp
               tests/unit/ics_builder_test.cpp
               tests/unit/ics_service_token_test.cpp
               tests/unit/app_workflow_test.cpp
               tests/unit/applications_service_test.cpp
               tests/unit/pdf_builder_test.cpp
               tests/unit/rooms_service_test.cpp
               tests/unit/notify_dispatcher_test.cpp
               tests/unit/reminder_service_test.cpp
               tests/unit/ws_registry_test.cpp
               tests/unit/staff_step_test.cpp
               tests/unit/notifications_feed_test.cpp
               tests/unit/people_service_test.cpp
               tests/unit/gradebook_rating_test.cpp
               tests/unit/library_service_test.cpp
               tests/unit/canteens_map_test.cpp
               src/controllers/HealthCtrl.cpp
               src/services/MigrationRunner.cpp
               src/services/OwnershipGuard.cpp
               src/services/PasswordHasher.cpp
               src/services/SessionService.cpp
               src/services/SettingsService.cpp
               src/services/ScheduleService.cpp
               src/services/ScheduleCache.cpp
               src/services/GradesService.cpp
               src/services/GradebookService.cpp
               src/services/RatingService.cpp
               src/services/AppWorkflow.cpp
               src/services/ApplicationsService.cpp
               src/services/PdfBuilder.cpp
               src/services/IcsService.cpp
               src/services/IcsBuilder.cpp
               src/services/RoomsService.cpp
               src/services/NotifyDispatcher.cpp
               src/services/ReminderService.cpp
               src/services/ScheduleChangeService.cpp
               src/services/StaffDashboard.cpp
               src/services/AnnouncementsService.cpp
               src/services/NotificationsFeedService.cpp
               src/services/WsRegistry.cpp
               src/services/PeopleService.cpp
               src/services/LibraryService.cpp
               src/services/CanteensService.cpp
               src/services/MapService.cpp
               src/filters/StaffFilter.cpp
               src/utils/LoginLimiter.cpp
               src/utils/NameMask.cpp)

target_include_directories(para-tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                                   ${CMAKE_CURRENT_SOURCE_DIR}/tests
                                   ${ARGON2_INCLUDE_DIR}
                                   ${HPDF_INCLUDE_DIR})
target_link_libraries(para-tests PRIVATE
                      ${PARA_DROGON_TARGETS}
                      ${PARA_SQLITE3_TARGET}
                      ${ARGON2_LIBRARY}
                      ${HPDF_LIBRARY}
                      ZLIB::ZLIB)

enable_testing()
add_test(NAME para-tests COMMAND para-tests)

# ##############################################################################
# security_tests — ПОЛНЫЙ HTTP-вариант автотеста безопасности 02 §9.3
# (Э8 шаг 1, DoD №2): поднимает ВЕСЬ сервер (весь PARA_APP_SOURCES — те же
# контроллеры/фильтры/сервисы, что в para, кроме main.cpp) в тестовом
# процессе на 127.0.0.1:<свободный порт> и ходит по нему реальным HTTP
# (drogon::HttpClient) с двумя живыми cookie-сессиями. Вывод теста
# прикладывается к отчёту жюри.
add_executable(security_tests
               tests/security_tests.cpp
               ${PARA_APP_SOURCES})

target_include_directories(security_tests
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
                                   ${CMAKE_CURRENT_SOURCE_DIR}/tests)
# Корень исходников в тесте: MigrationRunner::run(sql/migrations,
# sql/fixtures.sql) — пути резолвятся от источника независимо от cwd
# (ctest стартует в build/, ручной запуск — из корня проекта).
target_compile_definitions(security_tests PRIVATE
                           PARA_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# CSP-шаблоны нужны и тестовому серверу: блок CSRF шага 2 Э8 рендерит
# страницу GET /schedule (выдача токена) — без генерации
# newHttpViewResponse("schedule") отвечал бы 404-заглушкой Drogon
# (шаблон не зарегистрирован). Отдельный каталог generated-* — чтобы
# генерации para и security_tests не писали в одни файлы.
drogon_create_views(security_tests ${CMAKE_CURRENT_SOURCE_DIR}/views
                        ${CMAKE_CURRENT_BINARY_DIR}/generated-security-tests)
target_link_libraries(security_tests PRIVATE
                      ${PARA_DROGON_TARGETS}
                      ${PARA_SQLITE3_TARGET}
                      ${ARGON2_LIBRARY}
                      ${HPDF_LIBRARY}
                      ZLIB::ZLIB)
add_test(NAME security_tests COMMAND security_tests)
