#include "controllers/PeopleCtrl.h"

#include "services/PeopleService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Единая развязка вердикта сервиса → ответ (02 §5.1): Ok — 200 UTF-8
// (Э6 правки по гейту р1: emitUTF8 — сырая кириллица для гейт-поиска),
// BadRequest — 400 с текстом валидации, NotFound — 404, Error — 500
// (fail-closed, без деталей). 403 в people-роутах не возникает — см. шапку
// PeopleCtrl (правило видимости 02 §5.2: чужой студент — 200 урезанный).
void sendResult(std::function<void(const HttpResponsePtr&)>& cb,
                PeopleService::Result r)
{
    switch (r.status)
    {
        case PeopleService::Status::Ok:
            cb(api::jsonResponse(std::move(r.body)));
            return;
        case PeopleService::Status::BadRequest:
            cb(api::errorResponse("bad_request", r.error, k400BadRequest));
            return;
        case PeopleService::Status::NotFound:
            cb(api::errorResponse("not_found", "Человек не найден",
                                  k404NotFound));
            return;
        case PeopleService::Status::Error:
            cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                  k500InternalServerError));
            return;
    }
}

}  // namespace

void PeopleCtrl::search(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — факультет/роль запрашивающего сервис
    // берёт из БД по user_id сессии (02 §6.3: подмена запроса не работает).
    const auto session = req->attributes()->get<SessionInfo>("session");

    PeopleService::SearchQuery q;
    const std::string rawQ = req->getParameter("q");
    if (!rawQ.empty())
        q.q = rawQ;
    const std::string rawRole = req->getParameter("role");
    if (!rawRole.empty())
        q.role = rawRole;
    const std::string rawFaculty = req->getParameter("faculty");
    if (!rawFaculty.empty())
        q.faculty = rawFaculty;

    const auto db = app().getDbClient("para");
    PeopleService::loadRequester(
        db, session.userId,
        [db, q = std::move(q),
         cb = std::move(cb)](PeopleService::Status s,
                             PeopleService::Requester requester) mutable {
            if (s != PeopleService::Status::Ok)
            {
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
                return;
            }
            PeopleService::search(
                db, std::move(requester), std::move(q),
                [cb = std::move(cb)](PeopleService::Result r) mutable {
                    sendResult(cb, std::move(r));
                });
        });
}

void PeopleCtrl::person(const HttpRequestPtr& req,
                        std::function<void(const HttpResponsePtr&)>&& cb,
                        int64_t personId)
{
    const auto session = req->attributes()->get<SessionInfo>("session");
    PeopleService::profile(
        app().getDbClient("para"), session.userId, personId,
        [cb = std::move(cb)](PeopleService::Result r) mutable {
            sendResult(cb, std::move(r));
        });
}

void PeopleCtrl::schedule(const HttpRequestPtr&,
                          std::function<void(const HttpResponsePtr&)>&& cb,
                          int64_t personId)
{
    // requester не нужен: расписание группы студента публично (01 §5.11),
    // занятия преподавателя — фиксированная выборка по teacher_id.
    PeopleService::schedule(
        app().getDbClient("para"), personId,
        [cb = std::move(cb)](PeopleService::Result r) mutable {
            sendResult(cb, std::move(r));
        });
}
