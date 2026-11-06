#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para
{

// Заявки студента (Э5 шаг 2; 02 §5.2: POST/GET /v1/applications,
// GET /v1/applications/{id} «своя заявка + таймлайн», PUT — автосохранение
// черновика (С7: «случайно вышел из метро — введённое не потерялось»),
// POST {id}/messages — мини-диалог «требуется уточнение»; GET
// /v1/application-types — каталог). Вся SQL — параметризованная и живёт
// здесь (AGENTS.md §5.5, 02 §6.4); контроллер тонкий.
//
// Владение заявок с path-id проверяет КОНТРОЛЛЕР через
// OwnershipGuard::check(Table::Applications) ДО вызова любого метода с id —
// правило код-ревью в шапке OwnershipGuard.h (02 §6.3 п.3: чужое → 403,
// несуществующее → 404). Выборки по id здесь — по PK БЕЗ user_id-условия
// (паттерн gradeById Э4: владение проверяется ровно один раз, guard'ом).
// Список — всегда WHERE user_id = :session (02 §6.3 п.2).
//
// Событие подачи (NULL → submitted, автор — студент) пишется при POST с
// submit:true — как в fixtures.sql (у черновика событий нет); смена статуса
// и прочие переходы — через AppWorkflow::apply (матрица шага 1).
class ApplicationsService
{
  public:
    // Общий результат: ok → JSON 02 §5.2; !ok — ошибка БД → 500
    // (fail-closed, формат 02 §5.1).
    struct Result
    {
        bool ok = false;
        Json::Value body;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/application-types → {"types":[{key,title},…]} — из
    // constexpr-константы src/models/Applications.h (02 §4.3: каталог —
    // «constexpr-данные в коде»; состав карточек — 03 §4.5.2). Не ходит в БД.
    static Json::Value catalog();

    // Валидация тела POST (02 §5.2: body type, payload_json, submit) —
    // чистая функция; контроллер отдаёт 400 в формате 02 §5.1.
    //   type         — строка, ключ каталога kApplicationTypes (иначе 400);
    //   payload_json — опционален: строка с валидным JSON (иначе 400),
    //                  отсутствие → '{}' (DEFAULT DDL 02 §4.1);
    //   submit       — опционален: bool (иначе 400); false/отсутствие →
    //                  черновик, true → сразу submitted + событие подачи.
    struct CreateSpec
    {
        bool ok = false;
        std::string errorCode;
        std::string errorMessage;
        std::string type;
        std::string payloadJson;
        bool submit = false;
    };
    static CreateSpec parseCreate(const Json::Value& body);

    // POST /v1/applications (201): INSERT applications (draft | submitted)
    // + при submit — событие подачи NULL→submitted (fixtures.sql), всё в
    // одной транзакции (id новой строки — last_insert_rowid() на соединении
    // транзакции: между INSERT и SELECT на общем пуле мог вклиниться другой
    // запрос). body ответа — объект заявки (без таймлайна).
    static void create(drogon::orm::DbClientPtr db,
                       int64_t userId,
                       const CreateSpec& spec,
                       ResultCb cb);

    // GET /v1/applications (?status=…): свои заявки, новые сверху.
    // statusFilter: nullopt — все; значение обязано быть из словаря
    // kApplicationStatuses — проверяет контроллер (иначе 400 до сервиса).
    static void list(drogon::orm::DbClientPtr db,
                     int64_t userId,
                     const std::optional<std::string>& statusFilter,
                     ResultCb cb);

    // GET /v1/applications/{id} — ПОСЛЕ Own вердикта guard'а. Found —
    // карточка: поля заявки + payload (разобранный JSON) + timeline[]
    // (application_events по at,id — порядок записи). NotFound — после
    // Own guard'а строка исчезла (гонка удаления) → 404; Error → 500.
    enum class One
    {
        Found,
        NotFound,
        Error
    };
    struct OneResult
    {
        One status = One::NotFound;
        Json::Value body;
    };
    using OneCb = std::function<void(OneResult)>;

    static void one(drogon::orm::DbClientPtr db,
                    int64_t applicationId,
                    OneCb cb);

    // Валидация тела PUT (02 §5.2: body payload_json — автосохранение):
    // строка с валидным JSON (пустая строка — не JSON → 400).
    struct TextSpec
    {
        bool ok = false;
        std::string errorCode;
        std::string errorMessage;
        std::string text;
    };
    static TextSpec parsePayloadJson(const Json::Value& body);

    // PUT /v1/applications/{id} — ПОСЛЕ Own guard'а (guard ДО проверки
    // статуса: чужая не-draft заявка → 403, а не 409). ТОЛЬКО draft
    // (02 §5.2: «409 (статус ≠ draft)»): условный UPDATE … WHERE id AND
    // status='draft' — 0 строк → NotFound (заявки нет) или NotDraft (409).
    // Ok → body — обновлённая карточка (payload/status/updated_at).
    enum class Update
    {
        Ok,
        NotDraft,
        NotFound,
        Error
    };
    struct UpdateResult
    {
        Update status = Update::Error;
        Json::Value body;
    };
    using UpdateCb = std::function<void(UpdateResult)>;

    static void updateDraft(drogon::orm::DbClientPtr db,
                            int64_t applicationId,
                            const std::string& payloadJson,
                            UpdateCb cb);

    // Валидация тела POST messages (02 §5.2: body text): непустая строка.
    static TextSpec parseMessageText(const Json::Value& body);

    // POST /v1/applications/{id}/messages (201) — ПОСЛЕ Own guard'а.
    // Текст в таймлайн заявки событием с комментарием (С7), автор —
    // текущий пользователь. Ветка С7.6 (05 §3 Э5 п.3: «сообщение в
    // таймлайн, статус возвращается in_progress»): если заявка в
    // need_info — возврат в in_progress через AppWorkflow::apply (матрица
    // шага 1), текст — комментарий события перехода; при гонке (статус
    // ушли с need_info параллельно) — событие от фактического статуса,
    // сообщение не теряется. body ответа — объект события + статус
    // заявки ПОСЛЕ сообщения под ОБОИМИ именами — «application_status» и
    // «status» (С7.6 → in_progress; обычное сообщение — без смены; правки
    // по гейту Э5, раунды 1–2: автоматический возврат читаем прямо в
    // ответе).
    enum class Message
    {
        Ok,
        NotFound,
        Error
    };
    struct MessageResult
    {
        Message status = Message::Error;
        Json::Value body;
    };
    using MessageCb = std::function<void(MessageResult)>;

    static void sendMessage(drogon::orm::DbClientPtr db,
                            int64_t applicationId,
                            const std::string& text,
                            int64_t authorId,
                            MessageCb cb);

    // ============================== Э5 шаг 3 — staff =======================

    // GET /v1/staff/applications — очередь заявок (02 §5.2; С8.2: «фильтры
    // (тип, статус, срок); карточки с подходящим сроком помечены „горит“»).
    // Фильтры query — status (ключ словаря CHECK DDL), type (ключ каталога),
    // overdue=1: их валидность проверяет контроллер; здесь — одна
    // параметризованная SQL (filters-or-empty: «$1 = '' OR …» — без
    // конкатенации строк, 02 §6.4; порядок $N в тексте = порядку биндинга).
    // overdue («горит») — 3 дня ожидания в активном статусе (submitted/
    // accepted/in_progress/need_info; С2.4 «обычно до 3 рабочих дней» —
    // консервативно в календарных, бейдж загорается при приближении срока —
    // 03 §4.8); терминальные и draft не «горят». Сортировка —
    // a.updated_at ASC (очередь: дольше ждут — выше; idx_applications_queue).
    // Владение НЕ проверяется (staff видит все — постановка шага 3) —
    // это не student-owned выборка. В строке — карточка заявки + owner
    // (fio/группа — С8.3 «Открывает заявку Дины: профиль…») + overdue.
    struct QueueFilters
    {
        std::optional<std::string> status;
        std::optional<std::string> type;
        bool overdueOnly = false;
    };
    static void queue(drogon::orm::DbClientPtr db,
                      const QueueFilters& filters,
                      ResultCb cb);

    // POST /v1/staff/applications/{id}/status (02 §5.2: body to_status,
    // comment; «Смена статуса (автомат) + автоуведомление автору») —
    // применяет AppWorkflow::apply (матрица шага 1; недопустимый переход →
    // InvalidTransition → 409; несуществующий id → NotFound → 404 —
    // OwnershipGuard здесь не нужен: staff видит все, постановка шага 3).
    // Уведомление автору заявки (notifications, 02 §4.1: type 'application',
    // deep_link на карточку заявки) — в ТОЙ ЖЕ транзакции через хук
    // afterEvent. comment — причина (rejected) / вопрос (need_info).
    // Ok → body — карточка заявки в новом статусе.
    enum class StatusChange
    {
        Ok,
        InvalidTransition,
        NotFound,
        Error
    };
    struct StatusChangeResult
    {
        StatusChange status = StatusChange::Error;
        Json::Value body;
    };
    using StatusChangeCb = std::function<void(StatusChangeResult)>;

    static void staffChangeStatus(drogon::orm::DbClientPtr db,
                                  int64_t applicationId,
                                  const std::string& toStatus,
                                  const std::optional<std::string>& comment,
                                  int64_t staffUserId,
                                  StatusChangeCb cb);
};

}  // namespace para
