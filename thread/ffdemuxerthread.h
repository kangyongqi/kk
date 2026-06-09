#ifndef FFDEMUXERTHREAD_H
#define FFDEMUXERTHREAD_H

#include"ffthread.h"

#include<string>
#include<mutex>
#include<condition_variable>


extern "C"
{
#include"libavformat/avformat.h"
}

class FFDemuxer;
class FFAPacketQueue;
class FFVPacketQueue;
class FFPlayerContext;

class FFDemuxerThread : public FFThread
{
public:
    FFDemuxerThread();
    virtual ~FFDemuxerThread() override;

    void init(FFDemuxer* demuxer_);
    void wakeAllThread();
    void seek(int64_t seekSec);
    void close();

    bool peekStop();
protected:
    virtual void run() override;

private:
    void sendStopEvent();
private:
    FFDemuxer* demuxer = nullptr;
    FFPlayerContext* playerCtx = nullptr;
    std::condition_variable cond;
    std::mutex mutex;
    std::atomic<bool>stopFlag;
};


#endif // FFDEMUXERTHREAD_H



