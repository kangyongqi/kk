#ifndef FFOPENSOURCETHREAD_H
#define FFOPENSOURCETHREAD_H

#include"ffthread.h"
#include<queue>
#include<mutex>
#include<condition_variable>
#include<functional>
class FFEvent;


class FFOpenSourceThread : public FFThread
{
public:
    FFOpenSourceThread();

    void stop();
    void addEvent(FFEvent* event);

protected:
    virtual void run() override;
private:
    FFEvent* getEvent();

private:
    // 存储待处理的音视频操作事件
    std::queue<FFEvent*>evQueue;
    std::mutex mutex;
    std::condition_variable cond;
};

#endif // FFOPENSOURCETHREAD_H
