#include "ffeventqueue.h"
#include "event/ffevent.h"

FFEventQueue::~FFEventQueue() {
    std::lock_guard<std::mutex> lock(mutex);
    m_stop.store(true);
    cond.notify_all();
    while (!evQueue.empty()) {
        FFEvent* event = evQueue.front();
        evQueue.pop();
        delete event;
    }
}
//单例模式，全局共用一个evQueue
FFEventQueue &FFEventQueue::getInstance() {
    static FFEventQueue instance;
    return instance;
}

void FFEventQueue::enqueue(FFEvent* event) {
    std::lock_guard<std::mutex> lock(mutex);// 适用于简单的临界区保护，自动管理锁的生命周期
    evQueue.emplace(event);
//    std::cout<<"enqueue event!"<<std::endl;
    cond.notify_one(); // cond.notify_one() 是 C++ 标准库中 条件变量 的方法，用于 唤醒一个等待在该条件变量上的线程
}

FFEvent* FFEventQueue::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);// 适用于复杂的同步场景，特别是需要配合条件变量使用的情况
    // 调用 dequeue() 的线程检查返回值，如果为 nullptr ，退出循环，结束线程
    cond.wait(lock, [this]() { return !evQueue.empty() || m_stop.load(); });
    if (m_stop.load()) {
        return nullptr;
    }

    FFEvent* event = evQueue.front();
//    std::cout<<"dequeue event!"<<std::endl;
    evQueue.pop();
    return event;
}

void FFEventQueue::clearQueue()
{
    std::lock_guard<std::mutex> lock(mutex);
    while (!evQueue.empty()) {
        FFEvent* event = evQueue.front();
        evQueue.pop();
        delete event;
    }
}

void FFEventQueue::wakeAllThread() {
    std::lock_guard<std::mutex> lock(mutex);
    m_stop.store(true);
    cond.notify_all();
}
