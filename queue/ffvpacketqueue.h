#ifndef FFVPACKETQUEUE_H
#define FFVPACKETQUEUE_H

#include <condition_variable>
#include <mutex>
#include<queue>
#include<iostream>
#include<atomic>
#include<thread>
extern "C"
{
#include <libavformat/avformat.h>
}

class FFPacket;
class FFVPacketQueue
{
public:
    explicit FFVPacketQueue();
    ~FFVPacketQueue();

    FFPacket* dequeue();
    FFPacket *peekQueue();
    void enqueue(AVPacket* pkt);
    void enqueueFlush();
    void enqueueNull();
    void flushQueue();

    size_t getSerial();

    void wakeAllThread();
    void clearQueue();
    void close();
    void start();
    // 设置队列最大容量（线程安全）
    void setMaxSize(size_t maxSize);
private:
    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<size_t> serial;
    std::queue<FFPacket*>pktQueue;
    std::atomic<bool>m_stop;

    size_t maxSize = 2;
};

#endif // FFPACKETQUEUE_H
