#ifndef FFAPACKETQUEUE_H
#define FFAPACKETQUEUE_H

#include <condition_variable>
#include <mutex>
#include<queue>
#include<iostream>
#include<atomic>
extern "C"
{
#include <libavformat/avformat.h>
}



class FFPacket;

class FFAPacketQueue
{
public:
    explicit FFAPacketQueue();
    ~FFAPacketQueue();

    FFPacket* dequeue();
    FFPacket *peekQueue();
    void enqueue(AVPacket* pkt);
    void enqueueFlush();
    void enqueueNull();
    void flushQueue();

    size_t getSerial();
    void clearQueue();
    void wakeAllThread();
    void close();
    void start();

private:
    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<size_t> serial;
    std::queue<FFPacket*>pktQueue;
    std::atomic<bool>m_stop;


};

#endif // FFAPACKETQUEUE_H
