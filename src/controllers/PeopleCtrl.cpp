#include "controllers/PeopleCtrl.h"

#include "utils/I18nView.h"
#include "utils/StaffRouting.h"

#include <json/json.h>

#include <string>
#include <utility>

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

// --- Страничная плоскость (Э7 экран «Люди»; см. шапку заголовка) -----------

namespace
{

// Сериализация тел сервиса для view-данных — как в ApplicationsCtrl::page:
// сырая кириллица — emitUTF8; разбор и вывод с экранированием — шаблон
// (02 §6.4, Р-16).
std::string pageJson(const Json::Value& body)
{
    Json::StreamWriterBuilder w;
    w["emitUTF8"] = true;
    return Json::writeString(w, body);
}

// Каркасные view-данные страниц (partials head/topbar/tabbar): pageTitle и
// activeTab — серверные константы (02 §6.4). «Люди» — вложенный экран
// (вход из sheet «Детали пары» расписания и поиска), НЕ пункт таб-бара —
// пустой activeTab, как на /notifications.
// 5.18 (шаг 3): строки словаря — в view-данные ДО рендера (Р-16);
// fe_rebuild шаг 17: тела «Люди»/«Профиль человека» и заголовки вкладок
// тоже на словаре (people.* / person.*; pageTitleOf).
HttpViewData basePageData(std::string_view titleKey,
                          const HttpRequestPtr& req)
{
    HttpViewData data;
    const auto lang = i18n::resolveRequestLang(req);
    i18n::insertUiStrings(data, lang);
    para::insertStaffFlag(data, req);  // навигация сотрудника (Р-35)
    data.insert("pageTitle", i18n::pageTitleOf(titleKey, lang));
    data.insert("activeTab", std::string(""));
    return data;
}

// person.csp в одном из режимов view-переменной mode — data | notfound |
// dberror (паттерн application.csp). Код статуса на ответе — настоящий
// (200/404/500): страница с ошибкой — HTML-карточка, НЕ голый JSON.
// personId отдаётся шаблону/клиенту только ПОСЛЕ валидации цифр (используется
// блоком «Занятия» — GET /v1/people/{id}/schedule из app.js).
void sendPersonPage(std::function<void(const HttpResponsePtr&)> cb,
                    std::string mode,
                    HttpStatusCode code,
                    std::string personId,
                    std::string bodyJson,
                    const HttpRequestPtr& req)
{
    HttpViewData data = basePageData("person.title", req);
    data.insert("mode", std::move(mode));
    data.insert("personId", std::move(personId));
    if (!bodyJson.empty())
        data.insert("bodyJson", std::move(bodyJson));
    auto resp = HttpResponse::newHttpViewResponse("person", std::move(data));
    resp->setStatusCode(code);
    cb(std::move(resp));
}

}  // namespace

void PeopleCtrl::searchPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Страница без данных (паттерн notifications.csp): каталог-поиск грузит
    // клиент — GET /v1/people/search из app.js (тот же JSON-роут выше);
    // сессию проверил LoginFilter (303 без неё — страница, 02 §6.3).
    HttpViewData data = basePageData("people.title", req);
    cb(HttpResponse::newHttpViewResponse("people", std::move(data)));
}

void PeopleCtrl::personPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    const std::string& personId)
{
    const auto session = req->attributes()->get<SessionInfo>("session");

    // Path-id строкой: НЕЧИСЛОВОЙ id → 404 HTML-карточка (постановка
    // страницы; JSON-роут выше на нечисловом отвечает 400 средствами
    // drogon — контракты плоскостей разные, см. шапку). Только цифры и
    // влезание в int64_t — иначе нечисловой/переполнение → notfound.
    int64_t id = 0;
    bool numeric = !personId.empty() && personId.size() <= 18;
    if (numeric)
    {
        for (const char c : personId)
        {
            if (c < '0' || c > '9')
            {
                numeric = false;
                break;
            }
        }
        if (numeric)
        {
            try
            {
                id = std::stoll(personId);
            }
            catch (...)
            {
                numeric = false;  // переполнение — тот же notfound
            }
        }
    }
    if (!numeric)
    {
        sendPersonPage(std::move(cb), "notfound", k404NotFound, personId, "",
                   req);
        return;
    }

    // Path-id БЕЗ OwnershipGuard (документируется по постановке экрана):
    // people — общий каталог, 403 в people-роутах НЕ возникает (Р-15 в
    // docs/decisions.md; шапка PeopleService) — чужой студент легально
    // 200 с урезанными полями (маскирование имён на сервере, NameMask);
    // оценки/заявки/брони чужих people-эндпоинты не отдают вовсе, DoD №2
    // остаётся в силе на их собственных роутах (grades/applications/
    // room-bookings — guard с Э2/Э4/Э5а).
    PeopleService::profile(
        app().getDbClient("para"), session.userId, id,
        [cb = std::move(cb),
         personId,
         req](PeopleService::Result r) mutable {
            switch (r.status)
            {
                case PeopleService::Status::Ok:
                    // Разбор/ветвление по полям (роль, кабинет) — шаблон;
                    // сбой разбора шаблон покажет dberror-веткой (03 §5.3).
                    sendPersonPage(std::move(cb), "data", k200OK, personId,
                                   pageJson(r.body), req);
                    return;
                case PeopleService::Status::NotFound:
                case PeopleService::Status::BadRequest:
                    // BadRequest в profile не возвращается (только search) —
                    // защитно та же карточка notfound.
                    sendPersonPage(std::move(cb), "notfound", k404NotFound,
                                   personId, "", req);
                    return;
                case PeopleService::Status::Error:
                    // Ошибка БД — fail-closed (03 §5.3), 500 без деталей.
                    sendPersonPage(std::move(cb), "dberror",
                                   k500InternalServerError, personId, "", req);
                    return;
            }
        });
}
