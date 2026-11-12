#pragma once

// Мост словаря 5.18 (models/i18n.h) в рендер страниц — фича «Мультиязычный
// UI: ru/en/中文», шаг 3. Держит ПРАВИЛО Р-16 (docs/decisions.md): в CSP
// drogon v1.9.14 нет ни {{ }}, ни автоэкранирования — переведённые строки
// кладутся в HttpViewData ДО рендера (в рендер-вызове контроллера), шаблон
// только читает готовые значения. Значения словаря — серверные constexpr
// константы, экранировать на выходе нечего; пользовательские данные
// экранируются шаблонами htmlTranslate как раньше (02 §6.4).
//
// insertUiStrings кладёт:
//   "lang"     — код языка («ru»/«en»/«zh»): <html lang> в partials/head.csp
//                и ветки локального формата дат в шаблонах расписания;
//   "t.<key>"  — ВСЕ строки словаря (полный keyset, не список на страницу:
//                состав partials у каждой страницы свой, а частичные списки
//                пришлось бы синхронизировать с шаблонами руками; 328
//                коротких строк на рендер — копейки против пользы одного
//                источника);
//   "i18nJson" — тот же keyset JSON-объектом для static/js/app.js
//                (тосты/состояния клиента): footer.csp отдаёт его
//                <script type="application/json" id="para-i18n">, app.js
//                читает t(key, fallback). Значения — те же серверные
//                константы, JSON-экранирование минимально и выполняется
//                здесь же, до вставки (Р-16).
//
// Язык запроса — резолв шага 1 (i18n::resolveLang): cookie para_lang →
// Accept-Language → ru. СЛОВАРЬ НЕ ЗАВИСИТ от drogon (доступен юнит-тестам,
// шапка models/i18n.h) — зависимость от HttpRequest/HttpViewData живёт
// только в этом мосте.

#include <drogon/HttpRequest.h>
#include <drogon/HttpViewData.h>

#include "models/i18n.h"

namespace para::i18n
{

// Резолв языка запроса: cookie para_lang → заголовок Accept-Language → ru
// (defaultLang — русский, инвариант фичи 5.18).
Lang resolveRequestLang(const drogon::HttpRequestPtr& req);

// Кладёт в view-данные строки словаря текущего языка (см. шапку файла).
// Вызов — в рендер-вызове контроллера, ДО newHttpViewResponse (Р-16).
void insertUiStrings(drogon::HttpViewData& data, Lang lang);

// Заголовок вкладки «<экран> · Пара» из словаря: pageTitleOf("nav.schedule",
// Lang::En) → "Schedule · Para". Серверные константы — экранировать нечего.
std::string pageTitleOf(std::string_view screenKey, Lang lang);

}  // namespace para::i18n
