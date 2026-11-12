#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/SessionService.h"

namespace para {

// ============================================================================
// Новости и события университета — фича 5.17 (таблица news миграции 0007):
//
//   GET  /v1/news  — лента, видимая текущей сессии
//       (query: category, cursor)                          → 200 · 400;
//   публикация — БЕЗ нового staff-роута: kind='news' в
//   POST /v1/staff/announcements (StaffCtrl, 5.17 шаг 2)   → 201.
//
// SQL — только здесь, параметрами ($N; AGENTS §5.5, 02 §6.4). Формат
// ошибок — 02 §5.1, его собирает контроллер.
//
// Сущность отдельна от announcements (0003): объявления — authored-записи
// с JSON-аудиторией, новости — редакционная лента с категорией
// (official/conference/org/event — словарь CHECK 0007), временем события
// event_at и местом location. Записи news тоже БЕЗ push и БЕЗ уведомлений
// (как объявления, 01 §4.5): это лента, а не персональные оповещения —
// таймер/дайджест Э6 news не трогают.
//
// Аудитория новости — скаляр (не массив, в отличие от объявлений):
// audience 'all' | 'group' | 'course' + audience_value (id группы/курса,
// TEXT; NULL при 'all'). Видимость: 'all' — всем; 'group' — только своей
// группе, 'course' — только своему курсу (группа/курс из профиля сессии —
// тот же SELECT profiles, что AnnouncementsService::list); профиль
// отсутствует (staff, Р-6) → видит все — рабочий инструмент, прецедент
// AnnouncementsService::list.
//
// Пагинация курсором — СОСТАВНАЯ (published_at, id), РОВНО как у ленты
// уведомлений (NotificationsFeedService): published_at имеет секундную
// гранулярность, две новости одной секунды возможны; условие страницы:
//   published_at < $c OR (published_at = $c AND id < $cid),
// где $c/$cid — published_at/id строки-курсора. Курсор = id последней
// строки СВОЕЙ видимой страницы: сервис перечитывает её published_at по id
// С УСЛОВИЕМ ВИДИМОСТИ (у news нет владельца — «своя» строка = видимая
// этой аудитории); чужой по аудитории/несуществующий id → BadRequest
// (не раскрывает содержимое невидимых строк). Клиент передаёт next_cursor
// из ответа обратно в ?cursor=; next_cursor — id последней строки, только
// при наличии (kPageSize+1)-й — детектор следующей страницы.
//
// Сортировка published_at DESC, id DESC (стабильная при равных секундах).
//
// fail-closed: ошибка БД → Error → 500 без частичных данных; cb
// копируется в обе ветки каждой цепочки (урок Э2/Э5а, Э8 шаг 5).
// ============================================================================
class NewsService
{
  public:
    // Словарь CHECK 0007 — категория новости (единый для ленты и
    // публикации; пишет его же kind='news' из staff-роута).
    static bool isValidCategory(const std::string& category);

    // Параметры ленты (query GET /v1/news).
    struct Query
    {
        std::optional<std::string> category;  // ?category= — словарь CHECK 0007
        std::optional<int64_t> cursorId;      // ?cursor= — id последнего загр.
    };

    enum class FeedStatus
    {
        Ok,          // 200 — body готов
        BadRequest,  // 400 — недопустимая category / несуществующий курсор
        Error,       // 500 — ошибка БД (fail-closed)
    };
    struct FeedResult
    {
        FeedStatus status = FeedStatus::Error;
        Json::Value body;   // Ok: {"news":[…], "next_cursor":…}
        std::string error;  // BadRequest: человекочитаемая причина
    };
    using FeedCb = std::function<void(FeedResult)>;

    // Страница ленты (константа приложения, как kPageSize у соседних
    // лент): LIMIT pageSize+1 — строка-детектор следующей страницы.
    static constexpr std::size_t kPageSize = 20;

    // Лента, видимая сессии (аудитория — см. шапку класса).
    static void feed(drogon::orm::DbClientPtr db,
                     const SessionInfo& session,
                     const Query& q,
                     FeedCb cb);

    // --- Публикация (staff, kind='news' из POST /v1/staff/announcements) ---

    // Черновик новости: значения ПОЛНОСТЬЮ валидирует publish (словарь
    // category, строгий ISO 8601 UTC event_at, единичность аудитории) —
    // контроллер передаёт сырые строки тела, контракт ошибок тот же, что
    // у ленты (BadRequest + причина). body — непустая строка (общая
    // валидация title/body в StaffCtrl, прежняя для обоих kind).
    struct Draft
    {
        std::string category;                   // словарь CHECK 0007
        std::string title;
        std::string body;
        std::optional<std::string> eventAt;     // строгий YYYY-MM-DDTHH:MM:SSZ
        std::optional<std::string> location;    // «Корпус Б, ауд. 2105»
        // Аудитория: all → audience='all', value пуст; иначе ровно ОДНА
        // цель kind 'group'|'course' + её id (> 0) — скалярная схема
        // колонок 0007 (у объявления цели могут комбинироваться массивом,
        // у новости — нет: audience_value один).
        bool audienceAll = true;
        std::string audienceKind;
        int64_t audienceValue = 0;
    };

    enum class PublishStatus
    {
        Ok,          // 201 — body = объект новости
        BadRequest,  // 400 — невалидные category/event_at/location/audience
        Error,       // 500 — ошибка БД (fail-closed)
    };
    struct PublishResult
    {
        PublishStatus status = PublishStatus::Error;
        Json::Value body;   // Ok: объект новости (как строка ленты)
        std::string error;  // BadRequest: человекочитаемая причина
    };
    using PublishCb = std::function<void(PublishResult)>;

    // INSERT в news: published_at — серверное «сейчас» (strftime UTC —
    // тот же формат, что DDL 0004 read_at и SessionService::utcNow, DEFAULT
    // у 0007 сознательно нет — момент задаёт публикация); push/уведомлений
    // НЕТ (01 §4.5 — лента). id — перечитывание по естественному ключу
    // (урок Э5а: не last_insert_rowid — пул соединений).
    static void publish(drogon::orm::DbClientPtr db,
                        const Draft& draft,
                        PublishCb cb);
};

}  // namespace para
