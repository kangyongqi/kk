#ifndef FFAENCODER_H
#define FFAENCODER_H

#include<vector>
#include<mutex>

extern "C"
{
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
}

class FFAPacketQueue;

struct FFAEncoderPars{
    int sampleRate;
    int nbChannel;
    int biteRate;
    enum AVSampleFormat audioFmt;
};

struct PendingFrame {
    std::vector<uint8_t> data[8];
    int samples = 0;
    int64_t next_pts = 0;
};


class FFAEncoder
{


public:
    FFAEncoder();
    ~FFAEncoder();

    void init(FFAPacketQueue* pktQueue_);
    void close();
    void wakeAllThread();

    int encode(AVFrame* frame,int streamIndex,int64_t pts,AVRational timeBase);

    FFAEncoderPars* getEncoderPars();
    AVCodecContext* getCodecCtx();

    void initAudio(AVFrame* frame);
private:
    void printError(int ret);
    AVFrame *createFrameFromPending();
    void clearPendingFrame();

private:
    FFAPacketQueue* pktQueue = nullptr;
    AVCodecContext* codecCtx = nullptr;
    FFAEncoderPars* aPars = nullptr;

    std::mutex mutex;
    PendingFrame pendingFrame;

};

#endif // FFAENCODER_H
