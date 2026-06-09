#include"queue/ffeventqueue.h"
#include"thread/ffthreadpool.h"
#include"ffeventloop.h"
#include"ffevent.h"

void FFEventLoop::start() {
    m_stop = false;
    loopThread = std::thread(&FFEventLoop::work,this);
}

void FFEventLoop::stop()
{
    m_stop = true;

}

void FFEventLoop::wait()
{
    if(loopThread.joinable()){
        loopThread.join();
    }
}

void FFEventLoop::wakeAllThread()
{
    if(evQueue){
        evQueue->wakeAllThread();
    }
}


void FFEventLoop::work()
{
    while(!m_stop) {
        FFEvent* event = evQueue->dequeue();
        if(event == nullptr){
            continue;
        }

        threPool->submit([event]() mutable
        {
            std::cout<<"submit task"<<std::endl;
            event->work();
            delete event;
        });
    }
}



FFEventLoop::FFEventLoop()
    :m_stop(true)
{

}

FFEventLoop::~FFEventLoop()
{
    stop();
    if(threPool){
        delete threPool;
        threPool = nullptr;
    }
}

void FFEventLoop::init(FFEventQueue *evQueue_, FFThreadPool *threPool_)
{
    evQueue = evQueue_;
    threPool = threPool_;
}
