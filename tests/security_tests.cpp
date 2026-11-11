// security_tests — автотест безопасности 02 §9.3 (DoD №2: «проверка
// выполняется фактически, не декларируется»; вывод прикладывается к отчёту).
//
// Статус по шагам 02 §9.3 (честно — имитация успеха запрещена, AGENTS.md):
//   шаг 4 (grades/{id}: чужой → 403, свой → 200, несуществующий → 404) —
//     РЕАЛИЗОВАН здесь (Э4 шаг 3) на уровне OwnershipGuard::check(Table::Grades)
//     + GradesService::gradeById поверх временной SQLite с оценками
//     sql/fixtures.sql — тот же код, что под HTTP-контроллером
//     GradesCtrl::getGrade (верdict guard'а → тот же маппинг 403/404/500);
//   шаг 8 (бронь аудитории: чужая бронь DELETE → 403 — 01 §5.10 «бронь —
//     тоже личный объект») — РЕАЛИЗОВАН здесь (Э5а шаг 2) на уровне
//     OwnershipGuard::check(Table::RoomBookings) + RoomsService::create/
//     cancelById поверх временной SQLite — ровно та цепочка, что под
//     HTTP-контроллером RoomsCtrl::cancelBooking (guard ДО любых данных →
//     Own → сервис → 204); полный HTTP-вариант с двумя ЖИВЫМИ сессиями
//     (cookie) — Э8;
//   шаги 1–3, 5–7 (заявки/уведомления/staff/cookie) — Э8: их сущности и
//     эндпоинты появляются на Э5–Э6а, до того проверка этих строк —
//     вручную (curl с двумя сессиями).
//
// Две сессии — постановка Э4 шаг 3 (в §9.3 базовой редакции пары Аня/Олег;
// фикстуры v2: оценки добавлены ВСЕМ студентам, кроме Ани — ids 11–224,
// строки Марка не расширялись): user 1 = Аня (успеваемость ПУСТАЯ — 0 строк
// grades, средний null; предусловие С5.6/security-шага 4 держится и в v2),
// user 3 = Марк (как были ровно 10 оценок ids 1–10 — именно их дословно
// копирует мини-фикстура ниже, включая чужой для Ани id 1). «Сессия» на
// этом уровне — user_id,
// который контроллер берёт из атрибута LoginFilter (02 §6.3). Для шага 8 —
// та же пара §9.3: user 1 = Аня, user 2 = Олег (общая группа 1-ФМО —
// различие проверяем на ЛИЧНЫХ объектах).
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <future>
#include <string>

#include <sqlite3.h>

#include <drogon/orm/DbClient.h>

#include "services/GradesService.h"
#include "services/OwnershipGuard.h"
#include "services/RoomsService.h"

namespace fs = std::filesystem;
using para::GradesService;
using para::OwnershipGuard;
using para::RoomsService;

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

// Временная SQLite — как в tests/unit/grades_service_test.cpp (Э4 шаги 1–2):
// схема disciplines/grades без FK + строки оценок sql/fixtures.sql дословно
// (все 10 — user_id=3; фикстуры v2 их не расширяли, сверено sqlite);
// у Ани (user 1) оценок нет и в v2, session_plan не нужен
// (шаг 4 §9.3 — про grades/{id}).
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
};

void exec(sqlite3* db, const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK)
    {
        std::printf("FAIL  schema: %s\n", err ? err : "?");
        if (err != nullptr)
            sqlite3_free(err);
        std::exit(1);  // схема теста — предусловие, продолжать нет смысла
    }
}

void createSchema(const std::string& dbPath)
{
    sqlite3* raw = nullptr;
    if (sqlite3_open_v2(dbPath.c_str(),
                        &raw,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                        nullptr) != SQLITE_OK)
    {
        std::printf("FAIL  sqlite3_open(%s)\n", dbPath.c_str());
        std::exit(1);
    }
    exec(raw,
         "CREATE TABLE disciplines (id INTEGER PRIMARY KEY, code TEXT NOT"
         " NULL, title TEXT NOT NULL, control_type TEXT NOT NULL);"
         "INSERT INTO disciplines VALUES"
         " (1,'ME-101','Мировая экономика','exam'),"
         " (3,'HME-101','История МЭО','exam'),"
         " (6,'MT-401','Международная торговля','graded_credit'),"
         " (7,'EC-401','Эконометрика','exam'),"
         " (8,'BE-401','Деловой английский','credit'),"
         " (9,'MN-401','Международные переговоры','exam');"
         "CREATE TABLE grades ("
         "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  user_id INTEGER NOT NULL,"
         "  discipline_id INTEGER NOT NULL,"
         "  control_type TEXT NOT NULL,"
         "  component TEXT NOT NULL,"
         "  score INTEGER NOT NULL,"
         "  max_score INTEGER NOT NULL,"
         "  attempt INTEGER NOT NULL DEFAULT 1,"
         "  comment TEXT,"
         "  graded_at TEXT NOT NULL);"
         "CREATE INDEX idx_grades_user ON grades(user_id, discipline_id);"
         // Оценки sql/fixtures.sql дословно — все user_id=3 (Марк); v2
         // эти 10 строк не меняла (новые оценки v2 — ids 11–224, другие
         // пользователи — в мини-фикстуру не копируются).
         "INSERT INTO grades(id, user_id, discipline_id, control_type,"
         " component, score, max_score, attempt, comment, graded_at)"
         " VALUES"
         " (1,  3, 1, 'exam',          'модуль 1',  30, 30, 1, NULL,"
         "  '2026-10-05T12:00:00Z'),"
         " (2,  3, 1, 'exam',          'модуль 2',  30, 30, 1, NULL,"
         "  '2026-10-19T12:00:00Z'),"
         " (3,  3, 1, 'exam',          'семинары',  18, 22, 1, NULL,"
         "  '2026-10-31T12:00:00Z'),"
         " (4,  3, 6, 'graded_credit', 'модуль 1',  27, 30, 1, NULL,"
         "  '2026-10-08T12:00:00Z'),"
         " (5,  3, 6, 'graded_credit', 'реферат',   28, 30, 1, NULL,"
         "  '2026-10-22T12:00:00Z'),"
         " (6,  3, 7, 'exam',          'модуль 1',  24, 30, 1, NULL,"
         "  '2026-10-07T12:00:00Z'),"
         " (7,  3, 7, 'exam',          'модуль 2',  26, 30, 1, NULL,"
         "  '2026-10-21T12:00:00Z'),"
         " (8,  3, 8, 'credit',        'зачёт',     60, 60, 1, NULL,"
         "  '2026-10-14T12:00:00Z'),"
         " (9,  3, 9, 'exam',          'модуль 1',  25, 30, 1, NULL,"
         "  '2026-10-09T12:00:00Z'),"
         " (10, 3, 3, 'exam',          'экзамен',   25, 60, 1,"
         "  'Порог не достигнут; пересдача 18 ноября',"
         "  '2026-06-24T12:00:00Z');");
    // rooms/room_bookings (миграция 0005) — для шага 8 (бронь — тоже
    // личный объект): частичный UNIQUE idx_bookings_active включён,
    // схема цепочки create/cancel как в проде.
    exec(raw,
         "CREATE TABLE rooms ("
         "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  name TEXT NOT NULL,"
         "  building TEXT NOT NULL,"
         "  capacity INTEGER NOT NULL,"
         "  features TEXT NOT NULL DEFAULT '');"
         "INSERT INTO rooms(id, name, building, capacity, features) VALUES"
         " (1,'1207','Корпус А',24,'проектор, доска');"
         "CREATE TABLE room_bookings ("
         "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
         "  room_id INTEGER NOT NULL REFERENCES rooms(id),"
         "  user_id INTEGER NOT NULL,"
         "  starts_at TEXT NOT NULL,"
         "  ends_at TEXT NOT NULL,"
         "  status TEXT NOT NULL DEFAULT 'booked'"
         "    CHECK (status IN ('booked','cancelled','blocked')),"
         "  created_at TEXT NOT NULL DEFAULT"
         " (strftime('%Y-%m-%dT%H:%M:%SZ','now')));"
         "CREATE UNIQUE INDEX idx_bookings_active ON"
         " room_bookings(room_id, starts_at) WHERE status = 'booked';"
         "CREATE INDEX idx_bookings_user ON room_bookings(user_id,"
         " starts_at);");
    sqlite3_close(raw);
}

OwnershipGuard::Verdict guardGrade(const drogon::orm::DbClientPtr& db,
                                   int64_t sessionUserId,
                                   int64_t gradeId)
{
    // Ровно тот вызов, что делает GradesCtrl::getGrade до любых данных
    // (правило код-ревью OwnershipGuard.h); вердикты маппятся в HTTP:
    // Own → 200, NotFound → 404, Forbidden → 403, Error → 500.
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db,
                          OwnershipGuard::Table::Grades,
                          sessionUserId,
                          gradeId,
                          [&p](OwnershipGuard::Verdict v) { p.set_value(v); });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return OwnershipGuard::Verdict::Error;
    return f.get();
}

GradesService::OneResult gradeByIdOf(const drogon::orm::DbClientPtr& db,
                                     int64_t gradeId)
{
    std::promise<GradesService::OneResult> p;
    auto f = p.get_future();
    GradesService::gradeById(db,
                             gradeId,
                             [&p](GradesService::OneResult r) {
                                 p.set_value(std::move(r));
                             });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::OneResult{GradesService::One::Error, {}};
    return f.get();
}

GradesService::Result listOf(const drogon::orm::DbClientPtr& db, int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::list(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::Result{false, {}};
    return f.get();
}

GradesService::Result summaryOf(const drogon::orm::DbClientPtr& db,
                                int64_t userId)
{
    std::promise<GradesService::Result> p;
    auto f = p.get_future();
    GradesService::summary(db, userId, [&p](GradesService::Result r) {
        p.set_value(std::move(r));
    });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return GradesService::Result{false, {}};
    return f.get();
}

// --- Шаг 8 (Э5а шаг 2): цепочка DELETE /v1/room-bookings/{id} — ровно та,
// что под RoomsCtrl::cancelBooking: guard(Table::RoomBookings) ДО любых
// данных → Own → RoomsService::cancelById (по PK, без user_id-условия).
OwnershipGuard::Verdict guardBooking(const drogon::orm::DbClientPtr& db,
                                     int64_t sessionUserId,
                                     int64_t bookingId)
{
    std::promise<OwnershipGuard::Verdict> p;
    auto f = p.get_future();
    OwnershipGuard::check(db,
                          OwnershipGuard::Table::RoomBookings,
                          sessionUserId,
                          bookingId,
                          [&p](OwnershipGuard::Verdict v) { p.set_value(v); });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return OwnershipGuard::Verdict::Error;
    return f.get();
}

RoomsService::CreateResult createBookingOf(const drogon::orm::DbClientPtr& db,
                                           int64_t roomId,
                                           int64_t userId,
                                           const std::string& startsAt,
                                           const std::string& endsAt)
{
    std::promise<RoomsService::CreateResult> p;
    auto f = p.get_future();
    RoomsService::create(db,
                         roomId,
                         userId,
                         startsAt,
                         endsAt,
                         [&p](RoomsService::CreateResult r) {
                             p.set_value(std::move(r));
                         });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return RoomsService::CreateResult{RoomsService::CreateStatus::Error,
                                          {}};
    return f.get();
}

RoomsService::CancelStatus cancelOf(const drogon::orm::DbClientPtr& db,
                                    int64_t bookingId)
{
    std::promise<RoomsService::CancelStatus> p;
    auto f = p.get_future();
    RoomsService::cancelById(db, bookingId, [&p](RoomsService::CancelStatus s) {
        p.set_value(s);
    });
    if (f.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
        return RoomsService::CancelStatus::Error;
    return f.get();
}

}  // namespace

int main()
{
    std::printf("security_tests — автотест безопасности 02 §9.3 (DoD №2).\n");
    std::printf(
        "Статус: шаги 4 (grades/{id}) и 8 (room-bookings/{id}) — "
        "автопроверка; шаги 1–3, 5–7 — Э8.\n\n");

    // --- Сессия 1: Аня (user 1) — пустая успеваемость (состояние фикстур).
    // --- Сессия 2: Марк (user 3) — 10 оценок (ids 1–10).
    TempDir tmp;
    const std::string dbPath = (tmp.path / "para.db").string();
    createSchema(dbPath);
    auto db = drogon::orm::DbClient::newSqlite3Client("filename=" + dbPath, 1);

    std::printf("Шаг 4 (02 §9.3): grades/{id} — две сессии (Аня user 1 / "
                "Марк user 3).\n");

    // Предусловие сценария: успеваемость Ани пуста (иначе «чужой grade»
    // не определён), у Марка — ровно 10 оценок фикстур.
    const auto anyaList = listOf(db, 1);
    check(anyaList.ok && anyaList.body["grades"].size() == 0,
          "предусловие: список оценок Ани пуст (пустая успеваемость)");
    const auto anyaSummary = summaryOf(db, 1);
    check(anyaSummary.ok && anyaSummary.body["average"].isNull(),
          "предусловие: сводка Ани — average=null (пустое состояние)");
    const auto markList = listOf(db, 3);
    check(markList.ok && markList.body["grades"].size() == 10,
          "предусловие: у Марка 10 оценок (ids 1–10, фикстуры)");

    // Чужой grade → Forbidden (контроллер отвечает 403, формат 02 §5.1):
    // сессия Ани против КАЖДОЙ оценки Марка — не только id 1 из §9.3.
    bool foreignAllForbidden = true;
    for (int64_t id = 1; id <= 10; ++id)
    {
        if (guardGrade(db, 1, id) != OwnershipGuard::Verdict::Forbidden)
        {
            foreignAllForbidden = false;
            std::printf("        чужой grade id=%lld — не Forbidden\n",
                        static_cast<long long>(id));
        }
    }
    check(foreignAllForbidden,
          "чужой grade (Аня → ids 1..10 Марка) → Forbidden (403) — 10/10");

    // Свой grade → Own, затем данные (контроллер → 200 JSON).
    check(guardGrade(db, 3, 1) == OwnershipGuard::Verdict::Own,
          "свой grade (Марк → id 1) → Own");
    const auto own = gradeByIdOf(db, 1);
    check(own.status == GradesService::One::Found &&
              own.body["id"].as<int64_t>() == 1 &&
              own.body["score"].as<int64_t>() == 30,
          "свой grade (Марк → id 1) → 200/Ok (Found, score=30)");
    const auto ownRetake = gradeByIdOf(db, 10);
    check(ownRetake.status == GradesService::One::Found &&
              ownRetake.body["comment"].asString() ==
                  "Порог не достигнут; пересдача 18 ноября",
          "свой grade (Марк → id 10) → 200/Ok (неуд с комментарием)");

    // Несуществующий → NotFound (404), для ОБОИХ сессий — не маскируется
    // под Forbidden и не раскрывает чужие данные.
    check(guardGrade(db, 1, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующий grade (Аня → 999999) → NotFound (404)");
    check(guardGrade(db, 3, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующий grade (Марк → 999999) → NotFound (404)");

    // Fail-closed: ошибка БД — Error (контроллер → 500), не разрешение
    // (02 §6.3: чужое не отдаётся никогда).
    {
        TempDir broken;
        auto brokenDb = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + (broken.path / "para.db").string(), 1);
        check(guardGrade(brokenDb, 3, 1) == OwnershipGuard::Verdict::Error,
              "ошибка БД → Error (500, fail-closed — не Own)");
    }

    std::printf("\nИтог шага 4: %d/%d проверок пройдено.\n",
                g_checks - g_failed, g_checks);
    const int checks4 = g_checks;
    const int failed4 = g_failed;
    if (failed4 != 0)
    {
        std::printf("security_tests: ПРОВАЛ — найдены нарушения изоляции "
                    "grades/{id}.\n");
        return 1;
    }
    std::printf("security_tests: шаг 4 ПРОЙДЕН (чужой → 403, свой → 200, "
                "несуществующий → 404).\n");

    // ====================================================================
    // Шаг 8 (02 §9.3): «Олег вызывает DELETE /v1/room-bookings/{id_брони_ани}
    // → 403 (бронь аудитории — тоже личный объект)» (01 §5.10). Две сессии:
    // Аня user 1 / Олег user 2 (одна группа 1-ФМО — различие на личных
    // объектах). Цепочка контроллера: guard(Table::RoomBookings) → Own →
    // cancelById (204); маппинг вердиктов — 403/404/500.
    std::printf("\nШаг 8 (02 §9.3): room-bookings/{id} — две сессии "
                "(Аня user 1 / Олег user 2).\n");

    // Аня бронирует слот 13.11 (гейт-дата Э5а) через реальный сервис —
    // как POST /v1/rooms/1/bookings под контроллером. Фикстуры v2 дату
    // 13.11 по-прежнему не занимают (контроль: 0 броней на 2026-11-13).
    const auto anyaBooking =
        createBookingOf(db, 1, 1, "2026-11-13T10:00:00Z", "2026-11-13T11:00:00Z");
    check(anyaBooking.status == RoomsService::CreateStatus::Created &&
              anyaBooking.body["id"].as<int64_t>() > 0 &&
              anyaBooking.body["status"].asString() == "booked",
          "предусловие: Аня создала бронь 13.11 10:00–11:00 (Created, 201)");
    const int64_t idA = anyaBooking.body["id"].as<int64_t>();

    // Олег создаёт свою бронь другого слота (для проверки «свой → 204»).
    const auto olegBooking =
        createBookingOf(db, 1, 2, "2026-11-13T12:00:00Z", "2026-11-13T13:00:00Z");
    check(olegBooking.status == RoomsService::CreateStatus::Created,
          "предусловие: Олег создал свою бронь 13.11 12:00–13:00");
    const int64_t idB = olegBooking.body["id"].as<int64_t>();

    // Чужая бронь → Forbidden (контроллер → 403, 02 §6.3/§5.1) — сессия
    // Олега против брони Ани (п.8 §9.3 дословно) и наоборот.
    check(guardBooking(db, 2, idA) == OwnershipGuard::Verdict::Forbidden,
          "чужая бронь (Олег → id брони Ани) → Forbidden (403)");
    check(guardBooking(db, 1, idB) == OwnershipGuard::Verdict::Forbidden,
          "чужая бронь (Аня → id брони Олега) → Forbidden (403)");
    // Отмена чужой НЕ выполнялась (guard сработал до сервиса) — статус цел:
    check(guardBooking(db, 1, idA) == OwnershipGuard::Verdict::Own,
          "бронь Ани после чужого DELETE-попытки по-прежнему её (Own)");

    // Своя бронь → Own → отмена сервисом (204 No Content).
    check(guardBooking(db, 1, idA) == OwnershipGuard::Verdict::Own,
          "своя бронь (Аня → id_A) → Own");
    check(cancelOf(db, idA) == RoomsService::CancelStatus::Canceled,
          "своя бронь (Аня → id_A) → отмена Canceled (204)");
    check(guardBooking(db, 2, idB) == OwnershipGuard::Verdict::Own &&
              cancelOf(db, idB) == RoomsService::CancelStatus::Canceled,
          "своя бронь (Олег → id_B) → Own + Canceled (204)");

    // Повторная отмена уже отменённой — статус не booked (409 контроллером).
    check(cancelOf(db, idA) == RoomsService::CancelStatus::NotActive,
          "повторный DELETE своей отменённой → NotActive (409)");

    // Несуществующая → NotFound (404) для ОБОИХ сессий — не маскируется.
    check(guardBooking(db, 1, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующая бронь (Аня → 999999) → NotFound (404)");
    check(guardBooking(db, 2, 999999) == OwnershipGuard::Verdict::NotFound,
          "несуществующая бронь (Олег → 999999) → NotFound (404)");

    // Fail-closed: ошибка БД — Error (контроллер → 500), не разрешение.
    {
        TempDir broken;
        auto brokenDb = drogon::orm::DbClient::newSqlite3Client(
            "filename=" + (broken.path / "para.db").string(), 1);
        check(guardBooking(brokenDb, 2, idA) == OwnershipGuard::Verdict::Error,
              "ошибка БД (guard брони) → Error (500, fail-closed — не Own)");
        check(cancelOf(brokenDb, idA) == RoomsService::CancelStatus::Error,
              "ошибка БД (cancel) → Error (500, fail-closed)");
    }

    std::printf("\nИтог шага 8: %d/%d проверок пройдено.\n",
                (g_checks - checks4) - (g_failed - failed4),
                g_checks - checks4);
    if (g_failed != 0)
    {
        std::printf("security_tests: ПРОВАЛ — найдены нарушения изоляции "
                    "room-bookings/{id}.\n");
        return 1;
    }
    std::printf("security_tests: шаг 8 ПРОЙДЕН (чужая → 403, своя → 204, "
                "несуществующая → 404).\n");
    std::printf("security_tests: шаги 1–3, 5–7 автотеста 02 §9.3 — этап Э8.\n");
    return 0;
}
