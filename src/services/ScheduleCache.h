#pragma once

#include <cstdint>
#include <string>

#include <drogon/CacheMap.h>
#include <json/json.h>
#include <trantor/net/EventLoop.h>

namespace para {

// Модель недели группы — значение кэша ScheduleCache (гайд BE §5, Э3):
// события недели [from, to] в форме ответа 02 §5.2 (JSON-массив events).
// Ключ кэша — group_id: модель зависит только от данных группы (расписание
// группы — общее для её студентов, user_id в ключ не входит).
struct WeekModel
{
    int64_t groupId = 0;
    std::string from;  // понедельник недели, YYYY-MM-DD
    std::string to;    // воскресенье недели, YYYY-MM-DD
    Json::Value events = Json::Value(Json::arrayValue);
};

// Кэш моделей недели на drogon::CacheMap<group_id, WeekModel> (гайд BE §5,
// Э3): TTL 60 с — семантика CacheMap «пerase в пределах TTL от последнего
// обращения»; put по существующему ключу до истечения TTL — no-op (insert
// std::map не перезаписывает), поэтому попадание отдаёт модель той недели,
// что была закэширована, а неактуальность снимает только TTL или явная
// инвалидация. invalidate(group_id) — хук для Э6: внесение переноса/отмены
// инвалидирует кэш группы в контуре транзакции переноса (сама инвалидация —
// erase по ключу, не SQL).
class ScheduleCache
{
  public:
    static constexpr size_t kTtlSeconds = 60;

    // loop обязан жить дольше кэша (тикер CacheMap висит на runEvery).
    explicit ScheduleCache(trantor::EventLoop* loop);

    // Singleton прод-контура: loop приложения (app().getLoop()). Ленивая
    // инициализация потокобезопасна в любом потоке: CacheMap ставит тикер
    // через EventLoop::runEvery → TimerQueue::addTimer → runInLoop, который
    // вне потока loop'а уходит в очередь (проверено по исходникам
    // trantor 1.9.14: EventLoop.h runInLoop, TimerQueue.cc:193).
    static ScheduleCache& instance();

    // Попадание → true и копия модели (CacheMap::findAndFetch).
    bool get(int64_t groupId, WeekModel& out);

    // Записать модель группы с TTL kTtlSeconds.
    void put(int64_t groupId, WeekModel model);

    // Хук Э6: инвалидация при переносе (в контуре транзакции переноса).
    void invalidate(int64_t groupId);

  private:
    drogon::CacheMap<int64_t, WeekModel> map_;
};

}  // namespace para
