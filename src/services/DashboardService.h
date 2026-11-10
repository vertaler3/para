#pragma once

#include <functional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para {

// ============================================================================
// Агрегат дашборда «Сейчас важно» (Э7; 02 §5.2 GET /v1/dashboard, экран
// 03 §4.2). Логика агрегации живёт ЗДЕСЬ, а не в контроллере (AGENTS §3
// «контроллеры тонкие (10–30 строк), логика в src/services», §5.5; правки
// по ревью экрана — прежняя версия собирала агрегат в DashboardCtrl).
// Контроллер (DashboardCtrl) только вызывает dashboard() и мапит Result
// в 200/500 — SQL и бизнес-решений там нет.
//
// Источники — ТОЛЬКО публичные API существующих сервисов (нового SQL нет,
// src/services не расширялся новыми запросами):
//   profile.short_name — UserService::loadProfile. Имя = ВТОРОЕ слово fio:
//       формат фикстур «Фамилия Имя Отчество» (sql/fixtures.sql, согласован
//       с маской «А. Соколова» 02 §5.2 и рейтингом «М. Ливанов»), а
//       вайрфрейм 03 §4.2 приветствует ПО ИМЕНИ («Привет, Аня 👋»,
//       design-drafts/dashboard.html «Доброе утро, Аня.»); правки по
//       ревью — прежняя версия брала первое слово (фамилию). Однословный
//       fio → само слово. nullopt (staff без профиля Р-6 ИЛИ ошибка БД —
//       сервис не различает) → пустая строка, приветствие без имени.
//   today, next_class  — ScheduleService::today (день группы пользователя).
//       next_class = первая пара с ends_at > utcNow (лексикографическое
//       сравнение ISO-«…Z» — формат проекта, ScheduleService.h);
//       начавшаяся пара тоже «следующая» — FE пишет оверлайн
//       «ИДЁТ СЕЙЧАС · до HH:MM» (03 §4.2). Пар нет → null.
//   deadlines          — GradesService::exams (session_plan по starts_at),
//       первые 3 — «ближайшие» (03 §4.2). Прошедшие прячет FE.
//   applications       — ApplicationsService::list (свои, новые сверху) →
//       фильтр активных статусов (isActiveApplicationStatus: submitted/
//       accepted/in_progress/need_info/ready; draft не отправлен,
//       issued/rejected/cancelled — завершены), первые 3.
//   news, unread_count — NotificationsFeedService::feed: полная лента →
//       первые 3 (news); unread=1 → длина первой страницы (0..20;
//       «20+» при ≥20 рисует FE — правило бейджа каркаса).
//
// Формы объектов — ровно то, что отдают сервисы (слои не переизобретаются):
// today/next_class — события ScheduleService (eventJson), deadlines —
// session_plan GradesService::exams, applications — appJson, news —
// notificationJson (NotificationsFeedService.h).
//
// КОНТРАКТ ответа 200 (02 §5.2; базовые поля фиксированы постановкой
// экрана, degraded — аддитивное расширение по ревью):
//   {"profile":{"short_name":"Анна"},
//    "next_class":{…}|null,
//    "today":[…], "deadlines":[…], "applications":[…], "news":[…],
//    "unread_count":N,
//    "degraded":["today","news",…]}   — ключи источников, завершившихся
//       ошибкой БД (пустой массив, если все зелёные).
//
// Fail-soft виджета: ошибка БД источника деградирует ТОЛЬКО его — поле
// получает пустое значение (today=[], next_class=null, …), а ключ
// источника попадает в degraded; FE показывает компактную ошибку виджета
// с «Повторить» (03 §4.2 «каждый виджет загружается независимо… свой
// компактный ErrorState — сбой одного не роняет экран», §5.3 «ошибка
// внутри виджета дашборда — компактная строка, остальной экран живёт»;
// правки по ревью — прежняя версия молча отдавала пустые списки, и сбой
// был неотличим от «нет данных»). Общего 500 нет: доступ здесь не
// решается (path-id нет, все выборки — свои по user_id сессии, 02 §6.3
// п.2), OwnershipGuard не применим. Профиль в degraded НЕ попадает:
// loadProfile не различает «staff без профиля» и ошибку БД — пометка
// ложно срабатывала бы на staff; деградация приветствия (без имени)
// допустима по умолчанию.
// ============================================================================
class DashboardService
{
  public:
    struct Result
    {
        bool ok = false;  // false — нештатная ветка агрегации → 500 (fail-closed)
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // Активные статусы заявок виджета «Заявки» — словарь CHECK 0003:
    // черновик (не отправлена) и терминальные issued/rejected/cancelled
    // (работа завершена) — не «в работе» (03 §4.2: заявка в работе).
    static bool isActiveApplicationStatus(const std::string& status);

    // Собрать агрегат текущего пользователя (все источники — асинхронно,
    // параллельно; cb стреляет ровно один раз готовым body).
    static void dashboard(drogon::orm::DbClientPtr db, int64_t userId,
                          ResultCb cb);
};

}  // namespace para
