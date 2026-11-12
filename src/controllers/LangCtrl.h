#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Язык интерфейса (5.18, шаг 2): GET /lang?lang=ru|en|zh[&next=/path] —
// мини-роут установки cookie para_lang. ПУБЛИЧНЫЙ (без LoginFilter):
// язык выбирается и на экране входа (гость), и на любом экране — компактный
// переключатель в footer есть везде; сохранение в settings.lang — отдельный
// PUT /v1/notifications/settings {lang} из app.js (прецедент темы), здесь —
// только cookie: app.js cookie НЕ ставит (конвенция каркаса — все cookie
// выдаёт сервер: para_session/csrf). GET без CSRF — по матрице 02 §6.4
// (state-changing — POST/PUT/DELETE); смена собственной cookie языка не
// даёт атакующему выгоды: значения ограничены CHECK-списком ru|en|zh,
// барьер SameSite=Lax держит cross-site GET, ошибочная cookie исправляется
// следующим кликом по сегменту.
// Коды — те же, что CHECK миграции 0008, settings.lang и словарь
// src/models/i18n.h (langFromCode — единый источник списка кодов).
class LangCtrl : public HttpController<LangCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(LangCtrl::set, "/lang", Get);
    METHOD_LIST_END

    void set(const HttpRequestPtr& req,
             std::function<void(const HttpResponsePtr&)>&& cb);
};
