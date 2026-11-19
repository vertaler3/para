#include "utils/StaffRouting.h"

#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>

#include "services/SessionService.h"

namespace para
{
namespace
{
bool isStaffSession(const drogon::HttpRequestPtr& req)
{
    const auto& attrs = req->attributes();
    if (!attrs->find("session"))
        return false;
    return attrs->get<SessionInfo>("session").role == "staff";
}
}  // namespace

void insertStaffFlag(drogon::HttpViewData& data,
                     const drogon::HttpRequestPtr& req)
{
    if (isStaffSession(req))
        data.insert("staffEntry", std::string("1"));
}

bool isStudentOnlyPage(const std::string& path)
{
    static const char* const kExact[] = {"/schedule", "/grades",
                                         "/gradebook", "/rating",
                                         "/profile", "/applications"};
    for (const char* p : kExact)
        if (path == p)
            return true;
    return path.rfind("/grades/", 0) == 0 ||
           path.rfind("/applications/", 0) == 0;
}

void registerStaffPageRedirects()
{
    drogon::app().registerPreHandlingAdvice(
        [](const drogon::HttpRequestPtr& req, drogon::AdviceCallback&& acb,
           drogon::AdviceChainCallback&& accb) {
            if (req->method() == drogon::Get &&
                isStudentOnlyPage(req->path()) && isStaffSession(req))
            {
                acb(drogon::HttpResponse::newRedirectionResponse(
                    "/staff", drogon::k303SeeOther));
                return;
            }
            accb();
        });
}
}  // namespace para
