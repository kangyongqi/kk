#ifndef FFMUXER_H
#define FFMUXER_H

#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
extern "C" {
#include <libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libavutil/opt.h>
}

class FFMuxer {
public:
    FFMuxer();
    ~FFMuxer();

    void init(const std::string& url_,std::string const& format_ = "mp4");
    void addStream(AVCodecContext* codecCtx);
    int mux(AVPacket* packet);
    void writeHeader();
    void writeTrailer();

    int getAStreamIndex();
    int getVStreamIndex();

    void close();

private:
    void initMuxer();
    void printError(int ret);

private:
    std::string url;
    std::string format;

    AVFormatContext* fmtCtx = nullptr;
    AVCodecContext* aCodecCtx = nullptr;
    AVCodecContext* vCodecCtx = nullptr;

    AVStream* aStream = nullptr;
    AVStream* vStream = nullptr;

    int aStreamIndex = -1;
    int vStreamIndex = -1;

    bool headerFlag = false;
    bool trailerFlag = false;
    bool readyFlag = false;
    int streamCount = 0;

    std::mutex mutex;
};

#endif // FFMUXER_H
