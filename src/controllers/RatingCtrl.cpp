#include "controllers/RatingCtrl.h"

#include "services/PeopleService.h"
#include "services/RatingService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void RatingCtrl::show(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — факультет/роль запрашивающего сервис
    // берёт из БД по user_id сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");

    // query limit: опционален; нечисловой или вне 1..1000 → 400 (стиль
    // days в ScheduleCtrl: валидация аргумента — контроллер).
    int maxRows = 0;  // 0 — без ограничения
    const std::string rawLimit = req->getParameter("limit");
    if (!rawLimit.empty())
    {
        try
        {
            maxRows = std::stoi(rawLimit);
            if (maxRows < 1 || maxRows > 1000)
                throw std::out_of_range("limit");
        }
        catch (const std::exception&)
        {
            cb(api::errorResponse("bad_request",
                                  "limit — ожидается число от 1 до 1000",
                                  k400BadRequest));
            return;
        }
    }
    const std::string rawFaculty = req->getParameter("faculty");
    std::optional<std::string> faculty;
    if (!rawFaculty.empty())
        faculty = rawFaculty;

    const auto db = app().getDbClient("para");
    PeopleService::loadRequester(
        db, session.userId,
        [db, faculty = std::move(faculty), maxRows,
         cb = std::move(cb)](PeopleService::Status s,
                             PeopleService::Requester requester) mutable {
            if (s != PeopleService::Status::Ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            RatingService::rating(
                db, std::move(requester), faculty, maxRows,
                [cb = std::move(cb)](RatingService::Result r) mutable {
                    if (r.status == RatingService::Status::Ok)
                        cb(api::jsonResponse(std::move(r.body)));
                    else
                        cb(api::errorResponse(
                            "internal", "Внутренняя ошибка сервера",
                            k500InternalServerError));
                });
        });
}

// --- Страничная плоскость (Э7, экран «Рейтинг»; см. шапку .h) --------------

namespace {

// Сериализация тела сервиса для view-данных — как в GradesCtrl/ScheduleCtrl:
// сырая кириллица — emitUTF8; разбор и вывод с экранированием — шаблон
// (02 §6.4, Р-16).
std::string toJson(const Json::Value& body)
{
    Json::StreamWriterBuilder w;
    w["emitUTF8"] = true;
    return Json::writeString(w, body);
}

// Цепочка loadRequester → rating страницы — без query faculty/limit
// (общий рейтинг без обрезки — дефолт API, см. шапку .h); сбой БД —
// failed=1, шаблон рендерит состояние ошибки (03 §5.3).
void renderRatingPage(std::function<void(const HttpResponsePtr&)> cb,
                      const PeopleService::Requester& requester,
                      const drogon::orm::DbClientPtr& db)
{
    RatingService::rating(
        db, requester, std::nullopt, 0,
        [cb = std::move(cb)](RatingService::Result r) mutable {
            HttpViewData data;
            // Каркасные view-данные (partials head/topbar/tabbar) —
            // серверные константы (02 §6.4).
            data.insert("pageTitle", std::string("Рейтинг · Пара"));
            data.insert("activeTab", std::string("grades"));
            if (r.status == RatingService::Status::Ok)
            {
                // В view-данные — только сериализованное тело сервиса;
                // имена (маскированные или своё полное) экранирует шаблон
                // при выводе (02 §6.4, Р-15: рисуем как пришло).
                data.insert("failed", std::string("0"));
                data.insert("bodyJson", toJson(r.body));
            }
            else
            {
                // Ошибка БД при сборке страницы (03 §5.3): данных в
                // view-данные не кладём (паттерн GradesCtrl::gradesPage).
                data.insert("failed", std::string("1"));
            }
            cb(HttpResponse::newHttpViewResponse("rating", std::move(data)));
        });
}

}  // namespace

void RatingCtrl::ratingPage(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница, 02 §6.3);
    // роль/факультет запрашивающего сервис берёт из БД по user_id
    // сессии (02 §6.3) — как в JSON-роуте выше.
    const auto session = req->attributes()->get<SessionInfo>("session");
    const auto db = app().getDbClient("para");
    PeopleService::loadRequester(
        db, session.userId,
        [cb, db](PeopleService::Status s,
                 PeopleService::Requester requester) mutable {
            if (s != PeopleService::Status::Ok)
            {
                // Ошибка БД при загрузке запрашивающего — состояние
                // ошибки страницы, не данные (03 §5.3, fail-closed).
                HttpViewData data;
                data.insert("pageTitle", std::string("Рейтинг · Пара"));
                data.insert("activeTab", std::string("grades"));
                data.insert("failed", std::string("1"));
                cb(HttpResponse::newHttpViewResponse("rating",
                                                     std::move(data)));
                return;
            }
            renderRatingPage(std::move(cb), requester, db);
        });
}
