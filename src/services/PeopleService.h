#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

namespace para
{

// Каталог людей (5.11; 02 §5.2: GET /v1/people/search, /v1/people/{id},
// GET /v1/people/{id}/schedule; таблицы teacher_profiles 0006, users/
// profiles/groups 0001, schedule_events/disciplines 0002). Весь SQL —
// параметризованный и живёт здесь, не в контроллерах (AGENTS.md §5.5).
//
// ПРАВИЛО ВИДИМОСТИ (02 §5.2, строка «Правила видимости новых модулей
// (5.11–5.16)»; гайд BE §9а): преподаватель и сотрудник — полный профиль;
// студент — свой полный (поля /v1/me + почта), чужой — ТОЛЬКО публичные
// поля (имя, группа, почта) и расписание его группы; оценки, заявки и
// брони чужих не отдаются этими эндпоинтами вообще (в схемах ответов таких
// полей нет по построению) — DoD №2 (403 на чужой id) остаётся в силе на
// их собственных эндпоинтах (grades/applications/room-bookings, guard с
// Э2/Э4/Э5а). Следствие по кодам 02 §5.2: 403 в people-роутах НЕ
// возникает — просмотр профиля и расписания группы чужого студента легален
// (200 с урезанными полями); 404 — несуществующий/не-студент id; ошибки
// БД — 500 fail-closed.
//
// АДРЕСАЦИЯ /v1/people/{id} (Р-15 в docs/decisions.md): 02 §5.2
// id-пространство не определяет, а users.id и teacher_profiles.id
// пересекаются (студенты users 1–4, преподаватели teacher_profiles 1–6).
// Фиксируем: id < kPersonIdOffset — студент (users.id); id ≥
// kPersonIdOffset — преподаватель или сотрудник (teacher_profiles.id =
// id − kPersonIdOffset). Учетки сотрудников (dean.demo/office.demo, Р-14)
// в каталоге под своими users.id НЕ адресуются — их публичные карточки
// лежат в teacher_profiles (Р-14); users.id staff-а → NotFound.
//
// ПОЧТА: email есть только в teacher_profiles (0006); у profiles колонки
// email нет — почта СТУДЕНТА в каталоге вычисляется конвенцией
// login || '@example.edu' (Р-15; для сотрудников та же конвенция — их
// фикстурный email и есть login||'@example.edu', Р-14). Все адреса —
// вымышленный домен @example.edu (01 §5.11).
//
// ПОИСК (02 §5.2: query q, role=teacher|student|staff, faculty):
// преподаватели и сотрудники — видны всем; студенты — только своего
// факультета, если запрашивающий студент (запрашивающий staff — все
// студенты; факультет запрашивающего-студента берётся из БД по сессии,
// НЕ из query). role вне словаря → BadRequest (400 контроллера). q —
// подстрока ФИО (у людей — ещё title/department), спецсимволы LIKE
// экранируются (параметр, ESCAPE '\'); faculty-фильтр применяется только
// к студентам.
class PeopleService
{
  public:
    static constexpr int64_t kPersonIdOffset = 1000;

    // Почта студента в каталоге — конвенция Р-15 (см. шапку класса).
    static std::string studentEmail(const std::string& login)
    {
        return login + "@example.edu";
    }

    // Кто запрашивает: роль и факультет из БД по user_id сессии
    // (faculty/group — nullopt у staff, профиля нет). Error — ошибка БД.
    struct Requester
    {
        int64_t userId = 0;
        std::string role;  // 'student' | 'staff' (CHECK 0001)
        std::optional<std::string> faculty;
        std::optional<int64_t> groupId;
    };

    // --- Ответы (паттерн Result, как в RoomsService) ----------------------
    // BadRequest — валидация аргументов (role вне словаря); NotFound —
    // человека нет; Error — ошибка БД → 500 (fail-closed).
    enum class Status
    {
        Ok,
        BadRequest,
        NotFound,
        Error
    };
    struct Result
    {
        Status status = Status::Ok;
        Json::Value body;
        std::string error;  // текст для 400 (в 404/500 не отдаётся)
    };
    using ResultCb = std::function<void(Result)>;

    // Загрузить запрашивающего по user_id сессии (ОДИН запрос). Роль и
    // факультет — только из БД (02 §6.3): ограничение «студенты — своего
    // факультета» нельзя подменить параметром запроса. Пустой результат
    // (юзера сессии нет) — Error (fail-closed 500).
    static void loadRequester(drogon::orm::DbClientPtr db,
                              int64_t userId,
                              std::function<void(Status, Requester)> cb);

    // GET /v1/people/search — {"people": [...]}. Порядок: преподаватели,
    // затем сотрудники, затем студенты; внутри — по id.
    struct SearchQuery
    {
        std::optional<std::string> q;
        std::optional<std::string> role;  // teacher | student | staff
        std::optional<std::string> faculty;
    };
    static void search(drogon::orm::DbClientPtr db,
                       const Requester& requester,
                       const SearchQuery& q,
                       ResultCb cb);

    // GET /v1/people/{id} — профиль по правилу видимости (шапка класса).
    static void profile(drogon::orm::DbClientPtr db,
                        int64_t requesterUserId,
                        int64_t personId,
                        ResultCb cb);

    // GET /v1/people/{id}/schedule — занятия преподавателя (по teacher_id,
    // Р-13: teacher_profiles.id выровнен с teachers.id фикстурами) или
    // расписание группы студента (публично, 01 §5.11); сотрудник — пустой
    // список (занятий нет). События — форма schedule-ответов 02 §5.2
    // (ScheduleService::kSelect, включая teacher_id Э6а).
    static void schedule(drogon::orm::DbClientPtr db,
                         int64_t personId,
                         ResultCb cb);
};

}  // namespace para
