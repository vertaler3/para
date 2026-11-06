#include "controllers/PdfCtrl.h"

#include "models/Applications.h"
#include "services/ApplicationsService.h"
#include "services/PdfBuilder.h"
#include "services/UserService.h"
#include "utils/ApiError.h"

using namespace drogon;
using namespace para;

namespace {

// Вердикт guard'а → ответ (как в ApplicationsCtrl/GradesCtrl, 02 §6.3/§5.1).
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

void PdfCtrl::getPdf(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& cb,
                     int64_t applicationId)
{
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    auto db = app().getDbClient("para");
    // 1. Guard ДО любых данных и ДО проверки статуса (постановка шага 4:
    //    чужая не-ready заявка → 403, а не 409). userId — для выборки
    //    профиля после Own (Own означает app.user_id == session.user_id).
    const int64_t userId = session.userId;
    OwnershipGuard::check(
        db, OwnershipGuard::Table::Applications, session.userId, applicationId,
        [db, applicationId, userId,
         cb](OwnershipGuard::Verdict v) mutable {
            if (sendVerdict(cb, v))
                return;

            // 2. Заявка по PK (владение уже проверил guard).
            ApplicationsService::one(
                db, applicationId,
                [db, applicationId, userId,
                 cb](ApplicationsService::OneResult r) mutable {
                    if (r.status == ApplicationsService::One::NotFound)
                    {
                        cb(api::errorResponse("not_found", "Заявка не найдена",
                                              k404NotFound));
                        return;
                    }
                    if (r.status == ApplicationsService::One::Error)
                    {
                        cb(api::errorResponse("internal",
                                              "Внутренняя ошибка сервера",
                                              k500InternalServerError));
                        return;
                    }

                    // 3. Статус: PDF только для готовой/выданной
                    //    (02 §5.2: «409 (ещё не готова)»).
                    const std::string status =
                        r.body["status"].asString();
                    if (status != "ready" && status != "issued")
                    {
                        cb(api::errorResponse(
                            "not_ready", "Справка ещё не готова",
                            k409Conflict));
                        return;
                    }

                    // 4. Данные: карточка заявки + профиль владельца.
                    PdfBuilder::CertificateData data;
                    data.applicationId = applicationId;
                    data.typeTitle = r.body["type_title"].asString();
                    data.createdAt = r.body["created_at"].asString();
                    data.statusTitle = std::string(
                        models::applicationStatusTitle(status));
                    UserService::loadProfile(
                        db, userId,
                        [db, data, applicationId,
                         cb](std::optional<Json::Value> profile) mutable {
                            if (profile)
                            {
                                data.fio = (*profile).get("fio", "").asString();
                                data.faculty =
                                    (*profile).get("faculty", "").asString();
                                data.program =
                                    (*profile).get("program", "").asString();
                                data.course =
                                    (*profile).get("course", "").asString();
                                data.group =
                                    (*profile).get("group", "").asString();
                                data.studentCard =
                                    (*profile).get("student_card_no", "")
                                        .asString();
                            }

                            // 5. Сборка (шрифт — custom_config config.json).
                            const std::string fontPath =
                                app().getCustomConfig()
                                    .get("font_path",
                                         "/opt/para/fonts/DejaVuSans.ttf")
                                    .asString();
                            const auto built =
                                PdfBuilder::certificate(data, fontPath);
                            if (!built.ok)
                            {
                                // Детали (включая путь шрифта) — только в
                                // лог (02 §5.1).
                                LOG_ERROR << "PdfCtrl::getPdf(" << applicationId
                                          << "): " << built.error;
                                cb(api::errorResponse(
                                    "internal", "Внутренняя ошибка сервера",
                                    k500InternalServerError));
                                return;
                            }
                            auto resp = HttpResponse::newHttpResponse();
                            resp->setBody(built.bytes);
                            resp->setContentTypeString("application/pdf");
                            resp->addHeader(
                                "Content-Disposition",
                                "attachment; filename=spravka-" +
                                    std::to_string(applicationId) + ".pdf");
                            cb(std::move(resp));
                        });
                });
        });
}
