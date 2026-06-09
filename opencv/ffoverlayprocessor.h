#ifndef FFOVERLAYPROCESSOR_H
#define FFOVERLAYPROCESSOR_H

#include<opencv2/opencv.hpp>
#include"ffvideoadapter.h"

class FFOverlayProcessor
{
public:
    FFOverlayProcessor();

    void init();
    // 叠加FFmpeg格式的前景图像到背景画布
    void sendOverlayImage(AVFrame* frame,int x,int y,int w,int h);
    // 叠加OpenCV格式的前景图像到背景画布
    void sendOverlayImage(cv::Mat &mat,int x,int y,int w,int h);
    // 重置背景画布
    void resetBackground();
    // 获取叠加后的视频帧
    AVFrame* getOverlayFrame();
private:
/*

*/
    // 核心叠加逻辑（私有函数，仅内部调用）
    // foreground 前景图像Mat（待叠加的水印/UI元素）
    // 叠加位置X坐标（左上角）
    // 叠加位置Y坐标（左上角）
    // 叠加后的宽度（支持缩放）
    // 叠加后的高度（支持缩放）
    // 对前景图像进行缩放（匹配w/h）；
    // 校验叠加位置，避免越界（如x+w超过背景画布宽度）；
    // 执行图像叠加（支持透明通道的Alpha混合）；
    // 无返回值，直接修改background背景画布
    void overlayImage(cv::Mat &foreground,int x,int y,int w,int h);

private:
    // 存储叠加后的最终图像，是所有叠加操作的目标画布：
    cv::Mat background;
    FFVideoAdapter* adapter = nullptr;

    std::mutex mutex;
};

#endif // FFOVERLAYPROCESSOR_H
