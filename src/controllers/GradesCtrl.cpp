#include "controllers/GradesCtrl.h"

#include "utils/I18nView.h"

#include "services/GradesService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат сервиса → ответ: ok — JSON 02 §5.2, иначе единый формат ошибки
// 02 §5.1 (вложенный error.code/message) без деталей (500 — неожиданное,
// без стека наружу).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                GradesService::Result r)
{
    if (r.ok)
        cb(HttpResponse::newHttpJsonResponse(std::move(r.body)));
    else
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
}

// Вердикт guard'а → ответ (02 §6.3/§5.1): чужое — 403 forbidden, не
// существующее — 404 not_found, ошибка БД — 500 (fail-closed, разрешением
// не считается). Own обрабатывает вызывающий (там нужна выборка объекта).
bool sendVerdict(std::function<void(const HttpResponsePtr&)>& cb,
                 OwnershipGuard::Verdict v)
{
    switch (v)
    {
        case OwnershipGuard::Verdict::Own:
            return false;  // владение подтверждено — продолжить
        case OwnershipGuard::Verdict::NotFound:
            cb(api::errorResponse("not_found", "Запись не найдена",
                                  k404NotFound));
            return true;
        case OwnershipGuard::Verdict::Forbidden:
            // Формулировка — пример из 02 §5.1.
            cb(api::errorResponse("forbidden", "Доступ только к своим данным",
                                  k403Forbidden));
            return true;
        case OwnershipGuard::Verdict::Error:
            cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                  k500InternalServerError));
            return true;
    }
    return true;  // недостижимо при полном switch над enum
}

}  // namespace

void GradesCtrl::getGrades(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    GradesService::list(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradesService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void GradesCtrl::getSummary(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    GradesService::summary(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradesService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void GradesCtrl::getGrade(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb,
                          int64_t gradeId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Правило код-ревью (OwnershipGuard.h): path-id → guard ДО любой отдачи
    // данных. Guard различает NotFound/Forbidden; Own — единственный путь
    // к данным (gradeById — по PK, без user_id-условия: владение проверяется
    // ровно один раз, здесь). cb внешней лямбды — копия: guard стреляет один
    // раз, в Own-ветке её(move) забирает внутренняя лямбда выборки.
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Grades, session.userId, gradeId,
        [db, gradeId, cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            GradesService::gradeById(
                db, gradeId,
                [cb = std::move(cb)](GradesService::OneResult r) mutable {
                    switch (r.status)
                    {
                        case GradesService::One::Found:
                            cb(HttpResponse::newHttpJsonResponse(
                                std::move(r.body)));
                            return;
                        case GradesService::One::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Запись не найдена",
                                                  k404NotFound));
                            return;
                        case GradesService::One::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void GradesCtrl::getExams(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    GradesService::exams(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradesService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

// --- Страничная плоскость (Э7 экран «Успеваемость»; см. шапку заголовка) --

namespace {

// Сериализация тел сервиса для view-данных — как в ScheduleCtrl::page:
// сырая кириллица — emitUTF8; разбор и вывод с экранированием — шаблон
// (02 §6.4, Р-16).
std::string toJson(const Json::Value& body)
{
    Json::StreamWriterBuilder w;
    w["emitUTF8"] = true;
    return Json::writeString(w, body);
}

// Тела страницы успеваемости: сводка + список оценок + «Сессия» — те же
// методы GradesService, что стоят за JSON API (02 §5.2); сам сервис не
// правится (sql/ и src/services/ — BE-дорожка, AGENTS §5.8).
struct PageBodies
{
    Json::Value summary;
    Json::Value list;
    Json::Value exams;
};

// Цепочка summary → list → exams последовательно; колбэки копируются в
// КАЖДУЮ ветку (урок Э2 шаг 1: move в одной ветке оставил бы следующей
// пустой std::function → bad_function_call на error-пути БД).
void fetchGradesPage(const drogon::orm::DbClientPtr& db,
                     int64_t userId,
                     const std::function<void(PageBodies)>& onDone,
                     const std::function<void()>& onFail)
{
    GradesService::summary(
        db, userId,
        [db, userId, onDone,
         onFail](GradesService::Result r) {
            if (!r.ok)
            {
                onFail();
                return;
            }
            GradesService::list(
                db, userId,
                [db, userId, onDone, onFail,
                 summary = r.body](GradesService::Result r2) {
                    if (!r2.ok)
                    {
                        onFail();
                        return;
                    }
                    GradesService::exams(
                        db, userId,
                        [onDone, onFail,
                         summary = std::move(summary),
                         list = std::move(r2.body)](GradesService::Result r3) {
                            if (!r3.ok)
                            {
                                onFail();
                                return;
                            }
                            onDone(PageBodies{std::move(summary),
                                              std::move(list),
                                              std::move(r3.body)});
                        });
                });
        });
}

// Каркасные view-данные страниц (partials head/topbar/tabbar): pageTitle и
// activeTab — серверные константы (02 §6.4). 5.18 (шаг 3): строки словаря —
// в view-данные ДО рендера (Р-16); заголовок вкладки — из словаря
// («Успеваемость · Пара» / "Grades · Para" / «成绩 · Para»).
HttpViewData baseViewData(i18n::Lang lang)
{
    HttpViewData data;
    i18n::insertUiStrings(data, lang);
    data.insert("pageTitle", i18n::pageTitleOf("nav.grades", lang));
    data.insert("activeTab", std::string("grades"));
    return data;
}

}  // namespace

void GradesCtrl::gradesPage(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница, 02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // 5.18 (шаг 3): язык запроса резолвится ДО асинхронного рендера
    // (para_lang → Accept-Language → ru; Р-16 — строки в view-данные).
    const auto lang = i18n::resolveRequestLang(req);
    fetchGradesPage(
        app().getDbClient("para"), session.userId,
        [cb, lang](PageBodies b) mutable {
            HttpViewData data = baseViewData(lang);
            data.insert("failed", std::string("0"));
            // В view-данные — только сериализованные тела сервиса; строки
            // (дисциплины, комментарии) экранирует шаблон при выводе
            // (02 §6.4, Р-16).
            data.insert("summaryJson", toJson(b.summary));
            data.insert("listJson", toJson(b.list));
            data.insert("examsJson", toJson(b.exams));
            cb(HttpResponse::newHttpViewResponse("grades", std::move(data)));
        },
        [cb, lang]() mutable {
            // Ошибка БД при сборке страницы (03 §5.3): каркас с состоянием
            // ошибки и «Повторить» ([data-grades-retry] — app.js,
            // перезагрузка страницы); offline-баннер и ретрай сети — каркас
            // (topbar + app.js, 03 §5.4). Шаблон failed=1 рендерит только
            // блок ошибки — данных в view-данные не кладём.
            HttpViewData data = baseViewData(lang);
            data.insert("failed", std::string("1"));
            cb(HttpResponse::newHttpViewResponse("grades", std::move(data)));
        });
}

void GradesCtrl::disciplinePage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t disciplineId)
{
    // ВЛАДЕНИЕ НА ЭТОМ path-id — решение шага (см. шапку заголовка):
    // guard для path-id СТРОК оценки здесь не применяется; выборка своих
    // оценок по user_id сессии, чужой/несуществующий discipline_id → 404.
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // 5.18 (шаг 3): каркас страницы дисциплины — тот же бандл (тело экрана
    // остаётся по-русски — вторичный экран, вне охвата шага 3).
    const auto lang = i18n::resolveRequestLang(req);
    fetchGradesPage(
        app().getDbClient("para"), session.userId,
        [cb, disciplineId, lang](PageBodies b) mutable {
            // Дисциплина экрана — по СВОИМ оценкам (строки списка): нет ни
            // одной своей оценки с этим discipline_id — 404 not_found (и
            // чужая, и несуществующая — неотличимо, 03 §5.3).
            const Json::Value& grades = b.list["grades"];
            bool found = false;
            if (grades.isArray())
            {
                for (const auto& g : grades)
                {
                    if (g["discipline_id"].asInt64() == disciplineId)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (!found)
            {
                cb(api::errorResponse("not_found", "Запись не найдена",
                                      k404NotFound));
                return;
            }
            HttpViewData data = baseViewData(lang);
            data.insert("failed", std::string("0"));
            data.insert("disciplineId", std::to_string(disciplineId));
            data.insert("summaryJson", toJson(b.summary));
            data.insert("listJson", toJson(b.list));
            data.insert("examsJson", toJson(b.exams));
            cb(HttpResponse::newHttpViewResponse("grades_discipline",
                                                 std::move(data)));
        },
        [cb, disciplineId, lang]() mutable {
            // Ошибка БД — состояние ошибки (см. gradesPage), не данные.
            HttpViewData data = baseViewData(lang);
            data.insert("failed", std::string("1"));
            data.insert("disciplineId", std::to_string(disciplineId));
            cb(HttpResponse::newHttpViewResponse("grades_discipline",
                                                 std::move(data)));
        });
}
