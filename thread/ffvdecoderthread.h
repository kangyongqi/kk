#ifndef FFVDECODERTHREAD_H
#define FFVDECODERTHREAD_H


#include"ffthread.h"
extern "C"
{
#include<libavformat/avformat.h>
#include<libavutil/time.h>
}

class FFVPacketQueue;
class FFVDecoder;
class FFPlayerContext;

class FFVDecoderThread : public FFThread
{
public:
    FFVDecoderThread();
    virtual ~FFVDecoderThread() override;

    void init(FFVDecoder* vDecoder_,FFVPacketQueue* vPktQueue_);
    void wakeAllThread();
    void close();

    bool peekStop();
protected:
    virtual void run() override;

private:
    void sendEndEvent();
private:
    FFVPacketQueue* vPktQueue = nullptr;
    FFVDecoder* vDecoder = nullptr;
    FFPlayerContext* playerCtx = nullptr;
    std::atomic<bool>stopFlag;
};

#endif // FFVDECODERTHREAD_H
