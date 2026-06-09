#ifndef FFEVENTLOOP_H
#define FFEVENTLOOP_H

class FFEventQueue;
class FFThreadPool;

// 是事件的调度器 ，负责管理事件队列和线程，调用线程执行事件，相当于"事件的指挥官"
#include<thread>
#include<atomic>

class FFEventLoop {
public:
    explicit FFEventLoop();
    ~FFEventLoop();
    void init(FFEventQueue* evQueue_, FFThreadPool* threPool_);
    void start();
    void stop();
    void wait();

    void wakeAllThread();
private:
    void work();

private:
    FFEventQueue* evQueue;
    FFThreadPool* threPool;

    std::thread loopThread;
    std::atomic<bool> m_stop;

};


#endif // EVENT_LOOP_HPP
