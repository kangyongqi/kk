#ifndef FFARESAMPLER_H
#define FFARESAMPLER_H

extern "C"{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/channel_layout.h"
}
#include <iostream>

class FFAudioPars;
class FFAResampler{
public:
    explicit FFAResampler();
    ~FFAResampler();

    void init(FFAudioPars* src, FFAudioPars* dst);
    void resample(AVFrame* srcFrame, AVFrame** dstFrame);

private:
    void initSwr();
    AVFrame* allocFrame(FFAudioPars* aPars, int nbSamples, AVFrame* srcFrame);
    void printError(int ret);

private:
    SwrContext* swrCtx = nullptr;
    FFAudioPars* srcPars = nullptr;
    FFAudioPars* dstPars = nullptr;
    // 源声道布局
    // 告诉程序，音箱怎么摆放，声音从哪个方向出来
    AVChannelLayout srcLayout;
    AVChannelLayout dstLayout;
};

#endif // FFARESAMPLER_H
