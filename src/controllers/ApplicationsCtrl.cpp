#include "controllers/ApplicationsCtrl.h"

#include "utils/I18nView.h"

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Результат Result-сервисов → ответ: ok — JSON (опционально с кодом 201),
// иначе 500 без деталей (02 §5.1).
void sendResult(std::function<void(const HttpResponsePtr&)> cb,
                ApplicationsService::Result r,
                HttpStatusCode successCode = k200OK)
{
    if (!r.ok)
    {
        cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                              k500InternalServerError));
        return;
    }
    auto resp = HttpResponse::newHttpJsonResponse(std::move(r.body));
    resp->setStatusCode(successCode);
    cb(std::move(resp));
}

// Вердикт guard'а → ответ (02 §6.3/§5.1) — как в GradesCtrl: чужое — 403,
// несуществующее — 404, ошибка БД — 500 (fail-closed). Own → false, путь
// к данным продолжает вызывающий.
bool sendVerdict(std::function<void(const HttpResponsePtr&)>& cb,
                 OwnershipGuard::Verdict v)
{
    switch (v)
    {
        case OwnershipGuard::Verdict::Own:
            return false;
        case OwnershipGuard::Verdict::NotFound:
            cb(api::errorResponse("not_found", "Заявка не найдена",
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

void ApplicationsCtrl::getTypes(const HttpRequestPtr&,
                                std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Каталог — constexpr-константа src/models (02 §4.3); БД не нужна.
    cb(HttpResponse::newHttpJsonResponse(ApplicationsService::catalog()));
}

void ApplicationsCtrl::create(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    const auto json = req->getJsonObject();
    if (json == nullptr)
    {
        cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                              k400BadRequest));
        return;
    }
    const auto spec = ApplicationsService::parseCreate(*json);
    if (!spec.ok)
    {
        cb(api::errorResponse("bad_request", spec.errorMessage,
                              k400BadRequest));
        return;
    }
    ApplicationsService::create(
        app().getDbClient("para"), session.userId, spec,
        [cb = std::move(cb)](ApplicationsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r), k201Created);
        });
}

void ApplicationsCtrl::list(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& cb)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // query status — опциональный фильтр; значение сверяется со словарём
    // CHECK DDL (02 §4.1) — неизвестное → 400 (стиль days в ScheduleCtrl).
    std::optional<std::string> statusFilter;
    const std::string status = req->getParameter("status");
    if (!status.empty())
    {
        bool known = false;
        for (const auto& s : models::kApplicationStatuses)
            if (s == status)
            {
                known = true;
                break;
            }
        if (!known)
        {
            cb(api::errorResponse("bad_request",
                                  "status — недопустимое значение",
                                  k400BadRequest));
            return;
        }
        statusFilter = status;
    }
    ApplicationsService::list(
        app().getDbClient("para"), session.userId, statusFilter,
        [cb = std::move(cb)](ApplicationsService::Result r) mutable {
            sendResult(std::move(cb), std::move(r));
        });
}

void ApplicationsCtrl::getOne(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& cb,
                              int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Правило код-ревью (OwnershipGuard.h): path-id → guard ДО любой отдачи
    // данных (02 §6.3 п.3). Выборка после guard'а — по PK без user_id
    // (владение проверяется ровно один раз, здесь; паттерн getGrade Э4).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            ApplicationsService::one(
                db, applicationId,
                [cb = std::move(cb)](ApplicationsService::OneResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::One::Found:
                            cb(HttpResponse::newHttpJsonResponse(
                                std::move(r.body)));
                            return;
                        case ApplicationsService::One::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::One::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void ApplicationsCtrl::updateDraft(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Guard ДО разбора тела и ДО проверки статуса (постановка шага 2):
    // чужая не-draft заявка → 403, а не 409/400. req захватывается (сессия
    // проверена фильтром, тело читается после Own).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, req, userId = session.userId,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            const auto json = req->getJsonObject();
            if (json == nullptr)
            {
                cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                                      k400BadRequest));
                return;
            }
            const auto spec = ApplicationsService::parseUpdate(*json);
            if (!spec.ok)
            {
                cb(api::errorResponse("bad_request", spec.errorMessage,
                                      k400BadRequest));
                return;
            }
            ApplicationsService::updateDraft(
                db, applicationId, spec.payloadJson, spec.submit, userId,
                [cb = std::move(cb),
                 submit = spec.submit](
                    ApplicationsService::UpdateResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::Update::Ok:
                            cb(HttpResponse::newHttpJsonResponse(
                                std::move(r.body)));
                            return;
                        case ApplicationsService::Update::NotDraft:
                            // 02 §5.2: «409 (статус ≠ draft)» — и для
                            // правки, и для submit (Р-29/8.i: не-draft не
                            // подаётся, payload не тронут).
                            cb(api::errorResponse(
                                "invalid_status_transition",
                                submit
                                    ? "Подать можно только черновик заявки"
                                    : "Править можно только черновик заявки",
                                k409Conflict));
                            return;
                        case ApplicationsService::Update::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::Update::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

void ApplicationsCtrl::sendMessage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // Guard ДО разбора тела и ДО проверки статуса (постановка шага 2).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, req, userId = session.userId,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;
            const auto json = req->getJsonObject();
            if (json == nullptr)
            {
                cb(api::errorResponse("bad_request", "Ожидается JSON-тело",
                                      k400BadRequest));
                return;
            }
            const auto spec = ApplicationsService::parseMessageText(*json);
            if (!spec.ok)
            {
                cb(api::errorResponse("bad_request", spec.errorMessage,
                                      k400BadRequest));
                return;
            }
            ApplicationsService::sendMessage(
                db, applicationId, spec.text, userId,
                [cb = std::move(cb)](
                    ApplicationsService::MessageResult r) mutable {
                    switch (r.status)
                    {
                        case ApplicationsService::Message::Ok:
                        {
                            // 201 — событие диалога записано (02 §5.2);
                            // при need_info это событие перехода
                            // need_info→in_progress (С7.6).
                            auto resp = HttpResponse::newHttpJsonResponse(
                                std::move(r.body));
                            resp->setStatusCode(k201Created);
                            cb(std::move(resp));
                            return;
                        }
                        case ApplicationsService::Message::NotFound:
                            cb(api::errorResponse("not_found",
                                                  "Заявка не найдена",
                                                  k404NotFound));
                            return;
                        case ApplicationsService::Message::Error:
                            cb(api::errorResponse(
                                "internal", "Внутренняя ошибка сервера",
                                k500InternalServerError));
                            return;
                    }
                });
        });
}

// --- Страничная плоскость (Э7 экран «Заявки»; см. шапку заголовка) --------

namespace
{

// Сериализация тел сервиса для view-данных — как в ScheduleCtrl::page и
// GradesCtrl::gradesPage: сырая кириллица — emitUTF8; разбор и вывод
// с экранированием — шаблон (02 §6.4, Р-16).
std::string pageJson(const Json::Value& body)
{
    Json::StreamWriterBuilder w;
    w["emitUTF8"] = true;
    return Json::writeString(w, body);
}

// Каркасные view-данные страниц (partials head/topbar/tabbar): pageTitle и
// activeTab — серверные константы (02 §6.4). 5.18 (шаг 3): строки словаря —
// в view-данные ДО рендера (Р-16); заголовок передан готовым (список/форма
// переводятся, карточка заявки — русский заголовок, экран вне охвата шага).
HttpViewData basePageData(const std::string& title, i18n::Lang lang)
{
    HttpViewData data;
    i18n::insertUiStrings(data, lang);
    data.insert("pageTitle", title);
    data.insert("activeTab", std::string("applications"));
    return data;
}

}  // namespace

void ApplicationsCtrl::listPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница, 02 §6.3).
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // query status (v3, шаг 12): группа фильтра списка ИЛИ прежнее значение
    // словаря CHECK DDL — сводится к своей группе (ссылки ?status=need_info
    // продолжают работать). Сервис отдаёт ПОЛНЫЙ список: фильтр и поиск —
    // клиентские поверх SSR-карточек (app.js), группа только предвыбирает
    // фильтр и скрывает лишние карточки на SSR. НЕИЗВЕСТНОЕ → 303.
    const std::string status = req->getParameter("status");
    std::string group;
    if (!status.empty())
    {
        static const std::pair<const char*, const char*> kGroupOf[] = {
            {"active", "active"},       {"reply", "reply"},
            {"ready", "ready"},         {"issued", "issued"},
            {"draft", "draft"},         {"closed", "closed"},
            {"submitted", "active"},    {"accepted", "active"},
            {"in_progress", "active"},  {"need_info", "reply"},
            {"rejected", "closed"},     {"cancelled", "closed"}};
        for (const auto& g : kGroupOf)
            if (status == g.first)
            {
                group = g.second;
                break;
            }
        if (group.empty())
        {
            cb(HttpResponse::newRedirectionResponse("/applications",
                                                     k303SeeOther));
            return;
        }
    }
    ApplicationsService::list(
        app().getDbClient("para"), session.userId,
        std::nullopt,  // полный список (см. выше)
        [cb = std::move(cb), group,
         lang = i18n::resolveRequestLang(req)](
            ApplicationsService::Result r) mutable {
            // 5.18 (шаг 3): язык — para_lang → Accept-Language → ru;
            // строки словаря — в view-данные ДО рендера (Р-16).
            HttpViewData data =
                basePageData(i18n::pageTitleOf("apps.title", lang), lang);
            if (!r.ok)
            {
                // Ошибка БД при сборке страницы (03 §5.3): каркас с
                // состоянием ошибки и «Повторить» ([data-applications-retry]
                // — app.js, перезагрузка страницы); шаблон failed=1 рендерит
                // только блок ошибки — данных в view-данные не кладём.
                data.insert("failed", std::string("1"));
            }
            else
            {
                data.insert("failed", std::string("0"));
                data.insert("statusFilter", group);
                data.insert("bodyJson", pageJson(r.body));
            }
            cb(HttpResponse::newHttpViewResponse("applications",
                                                 std::move(data)));
        });
}

void ApplicationsCtrl::newPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter; path-id НЕТ — OwnershipGuard не применим
    // (правило — для эндпоинтов с {id} в пути, OwnershipGuard.h). Каталог
    // типов и профиль страница не хардкодит: их грузит клиент
    // (GET /v1/application-types и /v1/me — 02 §5.2), рендерит app.js;
    // состояния загрузки/ошибки — в шаблоне и app.js (03 §5.1/§5.3).
    // 5.18 (шаг 3): строки словаря — в view-данные ДО рендера (Р-16).
    const auto lang = i18n::resolveRequestLang(req);
    // v3 (шаг 12): ?type=<ключ> — deep link сразу в форму. Фаза — только
    // для первого кадра (заголовок h1 без мигания); валидность ключа
    // проверяет клиент по GET /v1/application-types (неизвестный ключ —
    // каталог), сервер ключ в разметку не выводит.
    const bool formPhase = !req->getParameter("type").empty();
    HttpViewData data = basePageData(
        i18n::pageTitleOf(formPhase ? "apps.docFormTitle" : "apps.catalogTitle",
                          lang),
        lang);
    data.insert("phase", std::string(formPhase ? "form" : "catalog"));
    // Имя шаблона — application_form (подчёркивание): drogon_ctl 1.9.14
    // генерирует из имени файла имя C++-класса, дефис не компилирует
    // (прецедент Э7 экрана «Расписание»: schedule_today/schedule_week).
    cb(HttpResponse::newHttpViewResponse("application_form", std::move(data)));
}

void ApplicationsCtrl::cardPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb,
    int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    // 5.18 (шаг 3): каркас карточки — тот же бандл (тело экрана
    // по-русски: карточка заявки — вторичный экран, вне охвата шага 3).
    const auto lang = i18n::resolveRequestLang(req);
    auto db = app().getDbClient("para");
    // РЕШЕНИЕ ПО GUARD на этом path-id (задокументировано в шапке
    // заголовка): OwnershipGuard::check(Table::Applications) ДО любой отдачи
    // данных (правило код-ревью в OwnershipGuard.h). Own — единственный путь
    // к данным (ApplicationsService::one — по PK без user_id-условия:
    // владение проверяется ровно один раз, здесь; паттерн getOne выше).
    // Для страницы Forbidden/NotFound → ОДНА нейтральная карточка ошибки
    // с РАЗНЫМИ кодами статуса 403/404 (03 §5.3: UI не различает
    // чужое/несуществующее; DoD 01 п.2 — 403 проверяем жюри); Error → 500
    // fail-closed. cb внешней лямбды — копия в КАЖДОЙ ветке (урок Э2 шаг 1:
    // move в одной ветке оставил бы следующей пустой std::function).
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, cb, lang](OwnershipGuard::Verdict v) mutable {
            switch (v)
            {
                case OwnershipGuard::Verdict::Own:
                    ApplicationsService::one(
                        db, applicationId,
                        [cb, lang](ApplicationsService::OneResult r) mutable {
                            HttpViewData data = basePageData("Заявка · Пара", lang);
                            HttpStatusCode code = k200OK;
                            switch (r.status)
                            {
                                case ApplicationsService::One::Found:
                                    data.insert("mode", std::string("data"));
                                    data.insert("bodyJson", pageJson(r.body));
                                    break;
                                case ApplicationsService::One::NotFound:
                                    // Строка исчезла после Own (гонка
                                    // удаления) — та же нейтральная
                                    // карточка, 404.
                                    data.insert("mode", std::string("guard"));
                                    code = k404NotFound;
                                    break;
                                case ApplicationsService::One::Error:
                                    data.insert("mode",
                                                std::string("dberror"));
                                    code = k500InternalServerError;
                                    break;
                            }
                            auto resp = HttpResponse::newHttpViewResponse(
                                "application", std::move(data));
                            resp->setStatusCode(code);
                            cb(std::move(resp));
                        });
                    return;
                case OwnershipGuard::Verdict::NotFound:
                case OwnershipGuard::Verdict::Forbidden:
                {
                    // Обе ситуации — одна нейтральная карточка «Не нашли то,
                    // что искали» (03 §5.3); коды статуса разные.
                    HttpViewData data = basePageData("Заявка · Пара", lang);
                    data.insert("mode", std::string("guard"));
                    auto resp = HttpResponse::newHttpViewResponse(
                        "application", std::move(data));
                    resp->setStatusCode(v == OwnershipGuard::Verdict::Forbidden
                                            ? k403Forbidden
                                            : k404NotFound);
                    cb(std::move(resp));
                    return;
                }
                case OwnershipGuard::Verdict::Error:
                {
                    // Ошибка БД — fail-closed (разрешением не считается):
                    // карточка «Сервис не отвечает», 500.
                    HttpViewData data = basePageData("Заявка · Пара", lang);
                    data.insert("mode", std::string("dberror"));
                    auto resp = HttpResponse::newHttpViewResponse(
                        "application", std::move(data));
                    resp->setStatusCode(k500InternalServerError);
                    cb(std::move(resp));
                    return;
                }
            }
        });
}
