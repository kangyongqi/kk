#ifndef FFDEMUXER_H
#define FFDEMUXER_H

#include<string>
#include<iostream>
#include<atomic>
#include<mutex>

extern "C" {
#include <libavformat/avformat.h>
#include<libavutil/cpu.h>
#include<libavdevice/avdevice.h>
#include<libavutil/time.h>
}

class FFVPacketQueue;
class FFAPacketQueue;

class FFDemuxer {
public:
    explicit FFDemuxer();
    FFDemuxer(const FFDemuxer&) = delete;
    FFDemuxer& operator=(const FFDemuxer&) = delete;
    ~FFDemuxer();

    void init(std::string const& url_,std::string const&format_,FFAPacketQueue* aPktQueue_,FFVPacketQueue* vPktQueue_,int type);
    int demux();
    AVStream* getAStream();
    AVStream* getVStream();
    void wakeAllThread();
    void close();
    void initDemuxer();
    int getType();
private:
    void printError(int ret);
private:
    std::string url;
    std::string format;

    AVFormatContext* fmtCtx = nullptr;
    AVDictionary* opts = nullptr;
    AVStream* aStream = nullptr;
    AVStream* vStream = nullptr;

    FFAPacketQueue* aPktQueue = nullptr;
    FFVPacketQueue* vPktQueue = nullptr;

    AVRational aTimeBase;
    AVRational vTimeBase;

    const AVInputFormat* inputFmt = nullptr;
    int type;
    bool offsetFlag = false;

    std::mutex mutex;
    bool stopFlag = false;
};

#endif // FF_DEMUXER_H
