#ifndef FFVFilter_H
#define FFVFilter_H

#include<iostream>
#include<thread>
#include<mutex>
#include<atomic>

extern "C"{
#include"libavformat/avformat.h"
#include<libavcodec/avcodec.h>
#include"libavfilter/avfilter.h"
#include <libavutil/opt.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include<libavutil/avutil.h>
}

class FFVFrameQueue;
class FFVDecoder;

class FFVFilter{
public:
    FFVFilter();
    ~FFVFilter();

    void init(FFVFrameQueue *encoderFrmQueue_,FFVDecoder* vDecoder1_,FFVDecoder* vDecoder2_);
    int sendFilter(AVFrame* frame1, AVFrame* frame2);

    AVRational getFrameRate();
    AVRational getTimeBase();
    enum AVMediaType getMediaType();
    void setOverlayPos(int x,int y,int w,int h);
    void sendEncodeFrame(AVFrame* frame);
private:
    void initFilter(AVCodecContext* codecCtx1_,AVCodecContext* codecCtx2_,AVStream* stream1_,AVStream* stream2_);
    void printError(int ret);

private:
    void createBufferFilter(AVFilterContext **ctx, AVCodecContext *codecCtx, AVStream *stream, const char *name);
    void linkFilters(AVFilterContext *src, int srcPad, AVFilterContext *dst, int dstPad);
    void createBufferSinkFilter();

    void getOverlayPos(int* x,int *y,int* w,int *h);

private:
//    FFVFrameQueue* frmQueue = nullptr;
    FFVFrameQueue* encoderFrmQueue = nullptr;
    FFVDecoder* vDecoder1 = nullptr;
    FFVDecoder* vDecoder2 = nullptr;

    AVCodecContext* codecCtx1 = nullptr;
    AVCodecContext* codecCtx2 = nullptr;

    AVStream* stream1 = nullptr;
    AVStream* stream2 = nullptr;

    AVFilterGraph* filterGraph = nullptr;
    AVFilterContext* bufferCtx1 = nullptr;
    AVFilterContext* bufferCtx2 = nullptr;
    AVFilterContext* bufferSinkCtx = nullptr;
    AVFilterContext* scaleCtx = nullptr;
    AVFilterContext* overlayCtx = nullptr;

    int overlayX,overlayY;
    int overlayW,overlayH;

    std::mutex filterMutex;
};

#endif // FFVFilter_H
