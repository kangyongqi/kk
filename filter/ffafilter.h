#ifndef FFAFilter_H
#define FFAFilter_H

#include<iostream>
#include<thread>
#include<sstream>
#include<mutex>

extern "C"{
#include"libavformat/avformat.h"
#include"libavfilter/avfilter.h"
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include<libavutil/time.h>
#include<libavcodec/avcodec.h>
}

//混音保持一致48kHz FLTP
#define FF_SAMPLE_RATE 48000
#define FF_SAMPLE_FMT AV_SAMPLE_FMT_FLTP

class FFAFrameQueue;
class FFADecoder;

class FFAFilter{
public:
    FFAFilter();
    ~FFAFilter();

    void init(FFAFrameQueue *encoderFrmQueue_,FFADecoder* aDecoder1_,FFADecoder* aDecoder2_);
    int sendFilter(AVFrame* frame1, AVFrame* frame2,int64_t start,int64_t pauseTime);

    AVRational getTimeBase();
    enum AVMediaType getMediaType();

    void setAudioVolume(double value);
    void setMicrophoneVolume(double value);
    void setVolume(double value1,double value2);

    void sendFrame(AVFrame* frame,int64_t start,int64_t pauseTime);
    void sendSingleFilter(AVFrame*frame,int64_t start,int64_t pauseTime,int type);
private:
    void initFilter(AVCodecContext* codecCtx1_,AVCodecContext* codecCtx2_,AVStream* stream1_,AVStream* stream2_);
    void printError(int ret);

    void linkFilters(AVFilterContext *src, int srcPad, AVFilterContext *dst, int dstPad);
    AVFilterContext *createFilter(AVFilterGraph* filterGraph,const AVFilter *filter, const char *name, const char *filterArgs);
    void createBufferSinkFilter(AVFilterGraph* filterGraph,AVFilterContext **ctx,const char *name);
    void createAfftdnFilter(AVFilterGraph* filterGraph);
    void createAmixFilter(AVFilterGraph* filterGraph);
    void createBufferFilter(AVFilterGraph* filterGraph,AVFilterContext **ctx, AVCodecContext *codecCtx, AVStream *stream, const char *name);
    void createVolumeFilter(AVFilterGraph* filterGraph,AVFilterContext **ctx, double gain, const char *name);


    void initSingleFilter(AVCodecContext* codecCtx,AVStream* stream,int type);
    void destroyFilterGraph(AVFilterGraph* filterGraph);
private:
    FFAFrameQueue* encoderFrmQueue = nullptr;
    FFADecoder* aDecoder1 = nullptr;
    FFADecoder* aDecoder2 = nullptr;

    AVCodecContext* codecCtx1 = nullptr;
    AVCodecContext* codecCtx2 = nullptr;

    AVStream* stream1 = nullptr;
    AVStream* stream2 = nullptr;

    AVFilterGraph* filterGraph = nullptr;
    AVFilterContext* bufferCtx1 = nullptr;
    AVFilterContext* bufferCtx2 = nullptr;
    AVFilterContext* bufferSinkCtx = nullptr;
    AVFilterContext* scaleCtx = nullptr;
    AVFilterContext* amixCtx = nullptr;
    AVFilterContext* afftdnCtx = nullptr;
    AVFilterContext* volumeCtx1 = nullptr;
    AVFilterContext* volumeCtx2 = nullptr;

    std::mutex mutex;

    double audioVolume = 1.0;
    double microphoneVolume = 1.0;

    //AUDIO = 0 ,MICROPHONE = 1;
    AVFilterGraph* singleFilterGraph[2] = {nullptr, nullptr};
    AVFilterContext* singleBufferCtx[2] = {nullptr,nullptr};
    AVFilterContext* singleBufferSinkCtx[2] = {nullptr,nullptr};
    AVFilterContext* singleVolumeCtx[2] = {nullptr, nullptr};

};

#endif // FFAFilter_H
