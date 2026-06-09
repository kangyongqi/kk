#ifndef FFMuxerThread_H
#define FFMuxerThread_H

#include"ffthread.h"
#include<mutex>
extern "C"{
#include<libavformat/avformat.h>
}

class FFAPacketQueue;
class FFVPacketQueue;
class FFMuxer;
class FFAEncoder;
class FFVEncoder;
class FFPacket;
class FFCaptureContext;

class FFMuxerThread: public FFThread
{
public:
    FFMuxerThread();
    ~FFMuxerThread() override;

    void init(FFAPacketQueue*aPktQueue_,FFVPacketQueue*vPktQueue_,FFMuxer* muxer_,FFAEncoder* aEncoder_,FFVEncoder* vEncoder_
              ,FFCaptureContext* captureCtx_);

    void close();
    void wakeAllThread();

protected:
    virtual void run() override;

private:
    void sendCaptureProcessEvent(double seconds);

private:
    FFAPacketQueue*aPktQueue = nullptr;
    FFVPacketQueue*vPktQueue = nullptr;

    FFMuxer* muxer = nullptr;

    FFVEncoder* vEncoder = nullptr;
    FFAEncoder* aEncoder = nullptr;

    AVRational vTimeBase;
    AVRational aTimeBase;

    std::mutex mutex;

    double lastProcessTime = 0;

    FFCaptureContext* captureCtx = nullptr;
};

#endif // FFMuxerThread_H
