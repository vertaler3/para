#include "utils/NotFoundPage.h"

#include <drogon/drogon.h>

#include <string_view>

#include "utils/I18nView.h"

namespace para
{
void registerNotFoundPage()
{
    auto previous = drogon::app().getCustomErrorHandler();
    drogon::app().setCustomErrorHandler(
        [previous](drogon::HttpStatusCode code,
                   const drogon::HttpRequestPtr& req) -> drogon::HttpResponsePtr {
            if (code != drogon::k404NotFound || !req)
                return previous(code, req);
            const std::string_view path = req->path();
            if (path.rfind("/v1/", 0) == 0 || path.rfind("/ws/", 0) == 0 ||
                path.rfind("/static/", 0) == 0)
                return previous(code, req);
            const auto lang = i18n::resolveRequestLang(req);
            drogon::HttpViewData data;
            i18n::insertUiStrings(data, lang);
            data.insert("pageTitle", i18n::pageTitleOf("notFound.pageTitle", lang));
            auto resp = drogon::HttpResponse::newHttpViewResponse("not_found", data);
            resp->setStatusCode(drogon::k404NotFound);
            return resp;
        });
}
}  // namespace para
