#include "filters/StaffFilter.h"

#include "services/SessionService.h"
#include "utils/ApiError.h"

void StaffFilter::doFilter(const HttpRequestPtr& req,
                           FilterCallback&& fcb,
                           FilterChainCallback&& fccb)
{
    // Роль — только из сессии, которую проверил LoginFilter (атрибут
    // "session"; из БД её тянет SessionService::findValid — JOIN users).
    // Параметр роли от клиента не принимается (02 §6.3: роль строится
    // на сервере). Отсутствие атрибута (фильтр вызван без LoginFilter —
    // ошибка конфигурации роута) — fail-closed: не staff.
    const auto session = req->attributes()->get<para::SessionInfo>("session");
    if (session.role != "staff")
    {
        fcb(para::api::errorResponse("forbidden",
                                     "Доступ только для сотрудников",
                                     k403Forbidden));
        return;
    }
    fccb();
}
