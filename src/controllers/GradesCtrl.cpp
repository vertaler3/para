#include "controllers/GradesCtrl.h"

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
