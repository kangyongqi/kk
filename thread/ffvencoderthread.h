#ifndef FFVEncoderThread_H
#define FFVEncoderThread_H

#include"ffthread.h"

extern "C"{
#include<libavformat/avformat.h>
}

class FFVEncoderPars;
class FFVEncoder;
class FFVFrameQueue;
class FFVideoPars;
class FFMuxer;
class FFVFilter;

class FFVEncoderThread : public FFThread
{
public:
    FFVEncoderThread();
    virtual ~FFVEncoderThread()override;

    void init(FFVFilter* vFilter_,FFVEncoder*vEncoder_,FFMuxer*muxer_,FFVFrameQueue* frmQueue_);
    void wakeAllThread();
    void close();

protected:
    virtual void run() override;
private:
    void initEncoder(AVFrame*frame);

private:
    FFVEncoder* vEncoder = nullptr;
    FFVFrameQueue* frmQueue = nullptr;
    FFMuxer* muxer = nullptr;

    int streamIndex = -1;
    FFVFilter* vFilter = nullptr;

    AVRational timeBase;
    AVRational frameRate;

    int64_t firstFramePts = 0;
    bool firstFrame = true;
};

#endif // FFVEncoderThread_H
