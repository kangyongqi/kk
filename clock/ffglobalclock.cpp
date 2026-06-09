#include "ffglobalclock.h"

FFGlobalClock::FFGlobalClock()
    :audioClock(0),videoClock(0),readyFlag(false)
{}

FFGlobalClock *FFGlobalClock::getInstance()
{
    static FFGlobalClock* clock = new FFGlobalClock();
    return clock;
}

int64_t FFGlobalClock::getAudioClock()
{
    return audioClock.load(std::memory_order_acquire);
}

int64_t FFGlobalClock::getVideoClock()
{
    return videoClock.load(std::memory_order_acquire);
}


void FFGlobalClock::setAudioClock(int64_t clock)
{
    audioClock.store(clock,std::memory_order_release);
}

void FFGlobalClock::setVideoClock(int64_t clock)
{
    videoClock.store(clock,std::memory_order_release);
}

void FFGlobalClock::initClock()
{
    audioClock.store(0,std::memory_order_relaxed);
    videoClock.store(0,std::memory_order_relaxed);
    clock = 0;
}

void FFGlobalClock::setReadyFlag()
{
    readyFlag.store(true,std::memory_order_acquire);
}

bool FFGlobalClock::getReadyFlag()
{
    return readyFlag.load(std::memory_order_release);
}

void FFGlobalClock::setClock(int64_t clock_)
{
    std::lock_guard<std::mutex>lock(mutex);
    clock = clock_;
}

int64_t FFGlobalClock::getClock()
{
    std::lock_guard<std::mutex>lock(mutex);
    return clock;
}



