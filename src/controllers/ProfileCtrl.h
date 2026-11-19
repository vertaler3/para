#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Профиль (Э7, экран 4 из 4; 03 §4.7, идеи 5.9/5.12 из 01):
//   GET /profile — SSR-страница views/profile.csp.
// Путь /profile НЕ публичный (LoginFilter.cpp: isPublic его не содержит)
// и не начинается с /v1/ (не API-плоскость): без валидной сессии фильтр
// сам отвечает страницам 303 → /login (02 §6.3, Р-20) — контроллер получает
// управление только с сессией, отдельно её не проверяет (паттерн
// SettingsCtrl/ScheduleCtrl; сессия странице не нужна — данных в шаблоне нет).
// path-id в роуте нет — OwnershipGuard не применим (правило — для эндпоинтов
// с {id} в пути). Данные экрана (/v1/me, /v1/library/my-card,
// /v1/notifications/settings, POST/DELETE /v1/ics/token, POST
// /v1/auth/logout) грузит static/js/app.js — пользовательских данных в
// шаблоне нет (паттерн views/rooms.csp), экранировать нечего (02 §6.4,
// Р-16: сырой вывод — только серверные константы pageTitle/activeTab и
// флаг icsState «active|none|unknown» — fe_rebuild шаг 16).
class ProfileCtrl : public HttpController<ProfileCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ProfileCtrl::page, "/profile", Get, "LoginFilter");
    METHOD_LIST_END

    void page(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
};
