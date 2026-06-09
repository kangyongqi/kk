#ifndef FFTHREAD_H
#define FFTHREAD_H

#include<thread>
#include<iostream>
#include<atomic>
#include<memory>

class FFThread{
public:
    explicit FFThread();

    virtual void run() = 0;
    virtual ~FFThread();
    void stop();
    void wait();
    void start();

protected:
    std::atomic<bool>m_stop;

private:
    std::thread m_thread;
};

#endif
