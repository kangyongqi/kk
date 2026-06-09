#ifndef FFVDECODER_H
#define FFVDECODER_H


#include<iostream>
extern "C"
{
#include<libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/cpu.h>
#include<libavutil/hwcontext.h>
}
#include<atomic>
#include<mutex>

class FFVFrameQueue;
class FFVResampler;

struct FFVideoPars{
    int width;
    int height;
    AVRational frameRate;
    AVPixelFormat pixFmtEnum;
    AVRational timeBase;
};

class FFVDecoder
{
public:
    explicit FFVDecoder();
    FFVDecoder(const FFVDecoder&) = delete;
    FFVDecoder& operator=(const FFVDecoder&) = delete;
    ~FFVDecoder();

    void decode(AVPacket* packet);
    void init(AVStream *stream_,FFVFrameQueue *frmQueue_);
    void flushDecoder();
    int getTotalSec();
    FFVideoPars* getVideoPars();

    AVCodecContext *getCodecCtx();
    AVStream *getStream();

    void enqueueNull();

    void wakeAllThread();
    void stop();
    void flushQueue();
    void close();
private:
    void initResampler();
    void initVideoPars(AVFrame* frame);
    void printError(int ret);

private:
    AVCodecContext* codecCtx = nullptr;
    AVStream* stream = nullptr;
    FFVFrameQueue* frmQueue = nullptr;
    FFVideoPars* vPars = nullptr;
    FFVideoPars* swsvPars = nullptr;
    FFVResampler* resampler = nullptr;

    bool hardDecodeFlag = false;

    std::atomic<bool>m_stop;

    std::mutex mutex;
};


#endif
