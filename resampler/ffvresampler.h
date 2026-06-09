#ifndef FFVRESAMPLER_H
#define FFVRESAMPLER_H

extern "C"{
#include"libavformat/avformat.h"
#include"libswscale/swscale.h"
#include"libavutil/imgutils.h"
}

class FFVideoPars;
class FFVResampler
{
public:
    FFVResampler();
    ~FFVResampler();

    void init(FFVideoPars* srcPars_, FFVideoPars* dstPars_);
    void resample(AVFrame* srcFrame,AVFrame** dstFrame);
private:
    AVFrame* allocFrame(FFVideoPars* vPars,AVFrame* srcFrame);
    void initSws();
    void printError(int ret);

private:
    SwsContext* swsCtx = nullptr;
    FFVideoPars* srcPars = nullptr;
    FFVideoPars* dstPars = nullptr;
    uint8_t* vBuffer = nullptr;
    int maxbufSize = -1;

};

#endif // FFVRESAMPLER_H
