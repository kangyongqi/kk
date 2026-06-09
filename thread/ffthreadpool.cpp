#include"thread/ffthreadPool.h"
#include<iostream>
FFThreadPool::FFThreadPool()
    :m_stop(false)
{
}

FFThreadPool::~FFThreadPool()
{
    stop();
    wait();
}

void FFThreadPool::init(size_t threadCount_)
{
    threadCount = threadCount_;
    for (size_t i = 0; i < threadCount; ++i) {
        threadVec.emplace_back(std::thread(&FFThreadPool::work,this));
    }
}

void FFThreadPool::work()
{
    m_stop = false;
    while(!m_stop){
        std::function<void()>task = getTask();
        if(task == nullptr){
            return;
        }
        std::cout<<"task!"<<std::endl;
        task();
    }
}

std::function<void()> FFThreadPool::getTask()
{
    std::unique_lock<std::mutex>lock(mutex);
    cond.wait(lock, [this]() { return m_stop || !taskQueue.empty(); });

    if(m_stop || taskQueue.empty()){
        return nullptr;
    }

    std::function<void()> task = taskQueue.front();
    taskQueue.pop();
    std::cout<<"getTask"<<std::endl;
    return task;
}

void FFThreadPool::enqueueTask(std::function<void()>task)
{
    std::lock_guard<std::mutex>lock(mutex);
    taskQueue.push(task);
    cond.notify_one();
}

void FFThreadPool::stop(){
    m_stop = true;
    cond.notify_all();

}

void FFThreadPool::wait()
{
    for(auto& thread : threadVec){
        // 检查这个工人是不是还在厂里上班，这个工人还在厂里吗？
        if(thread.joinable()){
            thread.join(); // 老板坐在门口等这个工人下班，工人不下班，老板不走
        }
    }
}
