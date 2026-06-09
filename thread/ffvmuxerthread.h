#ifndef FFVMuxerThread_H
#define FFVMuxerThread_H

#include"ffthread.h"

class FFVPacketQueue;
class FFMuxer;
class FFVMuxerThread: public FFThread
{
public:
    FFVMuxerThread();
    ~FFVMuxerThread() override;

    void init(FFVPacketQueue*pktQueue_,FFMuxer* muxer_);

protected:
    virtual void run() override;

private:
    void initMuxer();
private:
    FFVPacketQueue*pktQueue = nullptr;
    FFMuxer* muxer = nullptr;

    int64_t videoPts = 0;
    int64_t vFrameDuration = 0;

};

#endif // FFVMuxerThread_H
