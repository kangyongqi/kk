#ifndef FFVFRAMEQUEUE_H
#define FFVFRAMEQUEUE_H

#include <iostream>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>

extern "C" {
#include <libavformat/avformat.h>
}

class FFVFrameQueue {
public:
    explicit FFVFrameQueue();
    ~FFVFrameQueue();

    void enqueue(AVFrame* srcFrame);
    AVFrame* dequeue();

    void wakeAllThread();
    void clearQueue();
    void enqueueNull();
    void flushQueue();
    void close();
    void start();
    // 检查队列是否为空（线程安全）
    bool peekEmpty();

private:
    // 查看队首帧（不弹出，仅内部使用）
    AVFrame* peekQueue();
private:
    std::queue<AVFrame*> frmQueue;
    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<bool> m_stop;

};

#endif // FFVFRAMEQUEUE_H
