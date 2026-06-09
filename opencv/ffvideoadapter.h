#ifndef FFVIDEOADAPTER_H
#define FFVIDEOADAPTER_H

#include<opencv2/opencv.hpp>

extern "C"{
#include<libavformat/avformat.h>
#include<libswscale/swscale.h>
}

class FFVideoAdapter
{
public:
    FFVideoAdapter();
    ~FFVideoAdapter();

    AVFrame* convertMatToFrame(const cv::Mat& bgrMat);
    cv::Mat convertFrameToMat(AVFrame* frame);
private:
    void initSws(int width,int height);

    SwsContext* sws_ctx = nullptr;

    int lastW = 0,lastH = 0;
};

#endif // FFVIDEOADAPTER_H
