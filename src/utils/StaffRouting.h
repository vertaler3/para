#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpViewData.h>

// Роль сотрудника в каркасе страниц (фиксы 2026-11-19, Р-35).
//   insertStaffFlag — кладёт view-флаг staffEntry="1", если сессия
//     (атрибут LoginFilter "session") — staff. По флагу partials/topbar.csp
//     рисует навигацию сотрудника вместо студенческой. Роль — только из
//     серверной сессии (02 §6.3), не от клиента.
//   registerStaffPageRedirects — pre-handling advice (после фильтров, когда
//     сессия уже в атрибутах): GET студенческих SSR-страниц (расписание,
//     учёба, зачётка, рейтинг, профиль, заявки студента) сотруднику → 303
//     /staff. У сотрудника нет группы/профиля студента (Р-6) — эти экраны
//     у него пустые. API (/v1/*) не трогается. «/» — публичный путь без
//     атрибута сессии, его редиректит DashboardCtrl сам.
namespace para
{
void insertStaffFlag(drogon::HttpViewData& data,
                     const drogon::HttpRequestPtr& req);
bool isStudentOnlyPage(const std::string& path);
void registerStaffPageRedirects();
}  // namespace para
