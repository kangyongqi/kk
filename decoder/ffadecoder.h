#ifndef FFADECODER_H
#define FFADECODER_H

#include<iostream>
#include<atomic>
#include<mutex>
extern "C"
{
#include<libavformat/avformat.h>
#include<libavutil/samplefmt.h> // FFmpeg 的音频采样格式相关的类型、宏、函数
#include<libavcodec/avcodec.h>
}

struct FFAudioPars{
    int sampleRate;
    int nbChannels;
    AVRational timeBase;
    enum AVSampleFormat aFormatEnum;
    int sampleSize;
};

class FFAFrameQueue;
class FFAResampler;
class FFADecoder
{
public:
    explicit FFADecoder();
    FFADecoder(const FFADecoder&) = delete;
    FFADecoder& operator=(const FFADecoder&) = delete;
    ~FFADecoder();

    void decode(AVPacket* packet);
    void init(AVStream* stream_,FFAFrameQueue* frmQueue_);
    void flushDecoder();
    FFAudioPars* getAudioPars();
    int getToalSec();

    void wakeAllThread();
    void stop();
    void enqueueNull();
    void flushQueue();
    void close();

    AVCodecContext* getCodecCtx();
    AVStream* getStream();

private:
    void printError(int ret);
    void initAudioPars(AVFrame* frame);
    void initResampler();
    void printFmt();

private:
    AVCodecContext* codecCtx = nullptr;
    FFAFrameQueue* frmQueue = nullptr;
    AVStream* stream = nullptr;
    FFAudioPars* aPars = nullptr;
    FFAudioPars* swraPars = nullptr;
    FFAResampler* resampler = nullptr;
    std::atomic<bool>m_stop;

    std::mutex mutex;
};


#endif
