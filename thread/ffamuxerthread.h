#ifndef FFAMUXERTHREAD_H
#define FFAMUXERTHREAD_H

#include"ffthread.h"

class FFAPacketQueue;
class FFMuxer;
class FFAEncoder;

class FFAMuxerThread: public FFThread
{
public:
    FFAMuxerThread();
    ~FFAMuxerThread() override;

    void init(FFAPacketQueue*pktQueue_,FFMuxer* muxer_,FFAEncoder* aEncoder_);

protected:
    virtual void run() override;

private:
    FFAPacketQueue*pktQueue = nullptr;
    FFMuxer* muxer = nullptr;
    FFAEncoder* aEncoder = nullptr;
};

#endif // FFAMUXERTHREAD_H
