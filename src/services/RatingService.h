#pragma once

#include <functional>
#include <optional>
#include <string>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/PeopleService.h"  // Requester — роль/факультет запрашивающего

namespace para
{

// Рейтинг факультета (5.14; 02 §5.2: GET /v1/rating — «по среднему баллу;
// чужие имена маскируются на сервере», query faculty, limit). Студенты
// факультета по взвешенному среднему Σscore/Σmax×100 (целое — арифметика
// GradesService/GradebookService); студенты без оценок — в конце списка с
// average null.
//
// МАСКИРОВАНИЕ — НА СЕРВЕРЕ (02 §5.2, «Правила видимости 5.11–5.16»):
// «„А. Соколова“ для чужих, полное имя — только в строке самого
// пользователя (user_id сессии); полное имя чужих из БД наружу не уходит
// вовсе». SQL выбирает fio (маска строится на сервере функцией
// NameMask::maskFullName), но в JSON чужая строка содержит ТОЛЬКО маску —
// полное fio чужих в ответе отсутствует. Своей считается строка
// user_id == запрашивающего-студента; staff не студент — у него нет своей
// строки, все имена маскированы.
//
// ФАКУЛЬТЕТ (гейт Э6а раунд 1, уточнение Р-15): рейтинг ОБЩИЙ — все
// студенты (чужие маскированы, поэтому раскрытия данных нет); «Поиск
// студентов ограничен своим факультетом» (02 §5.2) касается
// /v1/people/search, не рейтинга — иначе «М. Ливанов» не виден студенту
// другого факультета, а гейт-чек требует маскированных чужих в списке.
// query faculty — опциональный фильтр для любого запрашивающего.
// limit валидирует контроллер (1..1000, нечисловой/вне диапазона → 400);
// обрезка — после сортировки, rank присваивается обрезанному списку.
class RatingService
{
  public:
    // Ok — 200; BadRequest — резерв (валидация limit в контроллере);
    // Error — ошибка БД → 500 (fail-closed).
    enum class Status
    {
        Ok,
        BadRequest,
        Error
    };
    struct Result
    {
        Status status = Status::Ok;
        Json::Value body;
        std::string error;
    };
    using ResultCb = std::function<void(Result)>;

    // GET /v1/rating → {"faculty": <название|null — все>, "rating":
    // [{rank, user_id, fio, average|null, self}], "masked": true}.
    // average — целое; null у студентов без оценок (сортировка: оценённые
    // по убыванию, затем по user_id; без оценок — в конец).
    static void rating(drogon::orm::DbClientPtr db,
                       const PeopleService::Requester& requester,
                       const std::optional<std::string>& faculty,
                       int maxRows,
                       ResultCb cb);
};

}  // namespace para
