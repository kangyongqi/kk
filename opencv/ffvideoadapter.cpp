#include "ffvideoadapter.h"

FFVideoAdapter::FFVideoAdapter()
{

}

FFVideoAdapter::~FFVideoAdapter()
{
    if(sws_ctx){
        sws_free_context(&sws_ctx);
    }
}

void FFVideoAdapter::initSws(int width, int height)
{
    if(sws_ctx){
        sws_free_context(&sws_ctx);
    }
    sws_ctx = sws_getContext(
                width, height, AV_PIX_FMT_BGR24,
                width, height, AV_PIX_FMT_YUV420P,
                SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
                );

    lastW = width;
    lastH = height;

    if (!sws_ctx) {
        std::cerr << "Error: Could not initialize SwsContext!" << std::endl;
        return;
    }
}


AVFrame *FFVideoAdapter::convertMatToFrame(const cv::Mat& bgrMat)
{
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "Error: Could not allocate video frame!" << std::endl;
        return nullptr;
    }

    int width = bgrMat.cols;
    int height = bgrMat.rows;
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = width;
    frame->height = height;

    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        std::cerr << "Error: Could not allocate the video frame data!" << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }

    int bgrLinesize[1] = { static_cast<int>(bgrMat.step) };
    const uint8_t* bgrData[1] = { bgrMat.data };

    if(sws_ctx == nullptr || lastW != width ||lastH != height){
        initSws(width,height);
    }
    sws_scale(
                sws_ctx, bgrData, bgrLinesize, 0, height,
                frame->data, frame->linesize
                );


    return frame;
}

/**
 * @brief 核心函数：FFmpeg YUV420P格式AVFrame → OpenCV BGR格式Mat
 * @param frame 输入的FFmpeg帧（必须是YUV420P格式，否则转换结果错误）
 * @return cv::Mat 转换后的BGR图像（8位3通道），OpenCV自动管理内存，无需手动释放
 * @details 核心逻辑：拼接YUV三平面数据→调用OpenCV cvtColor转换为BGR
 */
cv::Mat FFVideoAdapter::convertFrameToMat(AVFrame *frame)
{
    // 步骤1：获取输入帧的宽高
    int width = frame->width;
    int height = frame->height;

    // 步骤2：创建YUV420P格式的单通道Mat（关键尺寸说明）
    // YUV420P存储规则：
    // - Y平面（亮度）：width * height 个字节
    // - U平面（蓝色色度）：(width/2) * (height/2) = width*height/4 个字节
    // - V平面（红色色度）：width*height/4 个字节
    // 总尺寸 = width*height + width*height/4 + width*height/4 = width*height*3/2
    cv::Mat yuvMat(height * 3 / 2, width, CV_8UC1);

    // 步骤3：拷贝AVFrame的Y/U/V平面数据到yuvMat（按YUV420P顺序拼接）
    memcpy(yuvMat.data, frame->data[0], width * height);               // 拷贝Y平面（首地址开始）
    memcpy(yuvMat.data + width * height, frame->data[1], width * height / 4); // 拷贝U平面（Y平面之后）
    memcpy(yuvMat.data + width * height * 5 / 4, frame->data[2], width * height / 4); // 拷贝V平面（U平面之后）

    // 步骤4：OpenCV内置转换：YUV420P（I420格式）→ BGR24
    // COLOR_YUV2BGR_I420：对应YUV420P的标准转换公式
    cv::Mat bgrMat;
    cv::cvtColor(yuvMat, bgrMat, cv::COLOR_YUV2BGR_I420);

    // 步骤5：返回BGR格式Mat（OpenCV的Mat析构时自动释放内存）
    return bgrMat;
}


