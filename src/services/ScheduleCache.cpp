#include "services/ScheduleCache.h"

#include <drogon/drogon.h>

namespace para {

ScheduleCache::ScheduleCache(trantor::EventLoop* loop) : map_(loop)
{
}

ScheduleCache& ScheduleCache::instance()
{
    // Ленивый singleton: первый вызов — из обработчика запроса (IO-поток).
    // Magic static потокобезопасен; CacheMap::CacheMap ставит тикер через
    // runEvery → runInLoop — вне потока loop'а это очередь, а не assert
    // (EventLoop.h runInLoop; TimerQueue.cc addTimer → runInLoop).
    static ScheduleCache cache{drogon::app().getLoop()};
    return cache;
}

bool ScheduleCache::get(int64_t groupId, WeekModel& out)
{
    return map_.findAndFetch(groupId, out);
}

void ScheduleCache::put(int64_t groupId, WeekModel model)
{
    map_.insert(groupId, std::move(model), kTtlSeconds);
}

void ScheduleCache::invalidate(int64_t groupId)
{
    map_.erase(groupId);
}

}  // namespace para
