#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

// Лента объявлений деканата (Э6 шаг 4; 02 §5.2: GET /v1/announcements —
// «по аудитории своей группы/курса», query cursor). Тонкий контроллер:
// SQL — AnnouncementsService; объявления push-ем не отправляются (01 §4.5)
// — лента/дайджест. LoginFilter (сессия обязательна).
class AnnouncementsCtrl : public HttpController<AnnouncementsCtrl>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AnnouncementsCtrl::feed, "/v1/announcements", Get,
                  "LoginFilter");
    METHOD_LIST_END

    void feed(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& cb);
};
