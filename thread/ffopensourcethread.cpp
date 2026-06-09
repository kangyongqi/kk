#include "ffopensourcethread.h"
#include"event/ffopensourceevent.h"

FFOpenSourceThread::FFOpenSourceThread()
{

}

void FFOpenSourceThread::run()
{
    while(!m_stop){
        FFEvent* event = getEvent();
        if(event == nullptr){
            continue;
        }
        event->work();
        delete event;
    }
}

void FFOpenSourceThread::stop()
{
    m_stop = true;
}

FFEvent *FFOpenSourceThread::getEvent()
{
    std::unique_lock<std::mutex>lock(mutex);
    cond.wait(lock,[this](){ return m_stop || !evQueue.empty();});

    FFEvent* event = evQueue.front();
    evQueue.pop();

    return event;
}

void FFOpenSourceThread::addEvent(FFEvent *event)
{
    std::lock_guard<std::mutex>lock(mutex);
    evQueue.push(event);

    std::cerr << "add Event "<< std::endl;
    cond.notify_one();
}
