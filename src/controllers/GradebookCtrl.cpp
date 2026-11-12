#include "controllers/GradebookCtrl.h"

#include "utils/I18nView.h"
#include "services/GradebookService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

void GradebookCtrl::show(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");
    GradebookService::gradebook(
        app().getDbClient("para"), session.userId,
        [cb = std::move(cb)](GradebookService::Result r) mutable {
            // Ok — 200 UTF-8 (emitUTF8, Э6 правки р1); Error — 500
            // fail-closed, не пустая книжка.
            if (r.status == GradebookService::Status::Ok)
                cb(api::jsonResponse(std::move(r.body)));
            else
                cb(api::errorResponse("internal", "Внутренняя ошибка сервера",
                                      k500InternalServerError));
        });
}

// --- Страничная плоскость (Э7, экран «Зачётная книжка»; см. шапку .h) ------

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

}  // namespace

void GradebookCtrl::gradebookPage(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& cb)
{
    // Сессию проверил LoginFilter (303 без неё — страница, 02 §6.3);
    // книжка всегда своя — user_id только из сессии (02 §6.3).
    const auto session = req->attributes()->get<SessionInfo>("session");
    // 5.18 (шаг 3): каркас — словарь (тело зачётки — ru, вне охвата шага 3).
    const auto lang = i18n::resolveRequestLang(req);
    GradebookService::gradebook(
        app().getDbClient("para"), session.userId,
        [cb, lang](GradebookService::Result r) mutable {
            HttpViewData data;
            i18n::insertUiStrings(data, lang);
            // Каркасные view-данные (partials head/topbar/tabbar) —
            // серверные константы (02 §6.4).
            data.insert("pageTitle", std::string("Зачётная книжка · Пара"));
            data.insert("activeTab", std::string("grades"));
            if (r.status == GradebookService::Status::Ok)
            {
                // В view-данные — только сериализованное тело сервиса;
                // строки (дисциплины) экранирует шаблон при выводе
                // (02 §6.4, Р-16).
                data.insert("failed", std::string("0"));
                data.insert("bodyJson", toJson(r.body));
            }
            else
            {
                // Ошибка БД при сборке страницы (03 §5.3): шаблон с
                // failed=1 рендерит только блок ошибки с «Повторить»
                // — данных в view-данные не кладём (паттерн
                // GradesCtrl::gradesPage).
                data.insert("failed", std::string("1"));
            }
            cb(HttpResponse::newHttpViewResponse("gradebook", std::move(data)));
        });
}
