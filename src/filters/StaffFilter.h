#pragma once

#include <drogon/HttpFilter.h>

using namespace drogon;

// StaffFilter (02 §6.3 RBAC): на всех /v1/staff/* — роль пользователя
// сессии (users.role, CHECK 'student'|'staff', миграция 0001; в атрибут
// "session" кладёт LoginFilter из SessionService::findValid). Не staff →
// 403 в едином формате 02 §5.1 {error:{code,message}} — «студент, вызвавший
// staff-эндпоинт, получает 403» (шаг 7 security-автотеста 02 §9.3 — Э8).
// Вешается на роут ПОСЛЕ LoginFilter (в METHOD_LIST: "LoginFilter",
// "StaffFilter") — без атрибута сессии (LoginFilter не прошёл) фильтр
// fail-closed: не staff → 403. SQL не выполняет — решение по данным сессии.
// Механика регистрации — та же, что у LoginFilter (HttpFilter<T>,
// автосоздание по имени в METHOD_LIST; main.cpp — комментарий к запуску).
class StaffFilter : public HttpFilter<StaffFilter>
{
  public:
    void doFilter(const HttpRequestPtr& req,
                  FilterCallback&& fcb,
                  FilterChainCallback&& fccb) override;
};
