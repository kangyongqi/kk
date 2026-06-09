#include"ffvfilter.h"
#include"queue/ffvframequeue.h"
#include"decoder/ffvdecoder.h"
#include <mutex>

#define FF_VIDEO_FRAME_RATE { 30 , 1 }

#define FF_VIDEO_TIME_BASE { 1, 10000000 }
FFVFilter::FFVFilter() : overlayX(0),overlayY(0),overlayW(0),overlayH(0)
{
    overlayH = overlayY = overlayW = overlayH = 0;
}

FFVFilter::~FFVFilter()
{
    std::lock_guard<std::mutex> lock(filterMutex);
    if(filterGraph){
        avfilter_graph_free(&filterGraph);
    }
}

void FFVFilter::init(FFVFrameQueue *encoderFrmQueue_,
                     FFVDecoder *vDecoder1_, FFVDecoder *vDecoder2_)
{
    std::lock_guard<std::mutex> lock(filterMutex);
    encoderFrmQueue = encoderFrmQueue_;

    vDecoder1 = vDecoder1_;
    vDecoder2 = vDecoder2_;
}

//int FFVFilter::sendFilter(AVFrame *frame1, AVFrame *frame2)
//{
//    std::lock_guard<std::mutex> lock(filterMutex);

//    if(codecCtx1 == nullptr){
//        initFilter(vDecoder1->getCodecCtx(),vDecoder2->getCodecCtx(),
//                   vDecoder1->getStream(),vDecoder2->getStream());
//    }

//    int ret = av_buffersrc_add_frame_flags(bufferCtx1,frame1,AV_BUFFERSRC_FLAG_KEEP_REF);
//    if(ret < 0){
//        std::cerr << "Add Frame To BufferCtx1 Fail !" << std::endl;
//        printError(ret);
//        return -1;
//    }

//    ret = av_buffersrc_add_frame_flags(bufferCtx2,frame2,AV_BUFFERSRC_FLAG_KEEP_REF);
//    if(ret < 0){
//        std::cerr << "Add Frame To BufferCtx2 Fail !" << std::endl;
//        printError(ret);
//        return -1;
//    }

//    // 循环获取所有可用的输出帧
//    AVFrame* filterFrame = av_frame_alloc();
//    ret = av_buffersink_get_frame(bufferSinkCtx,filterFrame);
//    if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
//        av_frame_unref(filterFrame);
//        av_frame_free(&filterFrame);
//        return 0;
//    }
//    else if(ret < 0){
//        std::cerr << "Get BufferSinkCtx Frame Fail !" << std::endl;
//        printError(ret);
//        av_frame_unref(filterFrame);
//        av_frame_free(&filterFrame);
//        return -1;
//    }


//    encoderFrmQueue->enqueue(filterFrame);

//    av_frame_unref(filterFrame);
//    av_frame_free(&filterFrame);

//    return 0;
//}

AVRational FFVFilter::getFrameRate()
{
    return FF_VIDEO_FRAME_RATE;
}

AVRational FFVFilter::getTimeBase()
{
    return FF_VIDEO_TIME_BASE;
}

AVMediaType FFVFilter::getMediaType()
{
  return AVMEDIA_TYPE_VIDEO;
}

//void FFVFilter::initFilter(AVCodecContext *codecCtx1_, AVCodecContext *codecCtx2_,
//                           AVStream *stream1_, AVStream *stream2_) {
//    codecCtx1 = codecCtx1_;
//    codecCtx2 = codecCtx2_;
//    stream1 = stream1_;
//    stream2 = stream2_;

//    //暂时直接退出
//    return;

//    // 创建滤镜图
//    filterGraph = avfilter_graph_alloc();
//    if (!filterGraph) {
//        std::cerr << "Alloc Filter Graph Fail!" << std::endl;
//        return;
//    }

//    // 初始化输入缓冲区滤镜
//    createBufferFilter(&bufferCtx1, codecCtx1, stream1, "in1");

//    // 初始化输出sink滤镜
//    createBufferSinkFilter();

//    // 链接滤镜节点
//    linkFilters(bufferCtx1, 0, bufferSinkCtx, 0);    // main输入 -> overlay主输入

//    // 启用自动� �式转换
//    avfilter_graph_set_auto_convert(filterGraph, AVFILTER_AUTO_CONVERT_ALL);

//    // 配置滤镜图
//    int ret = avfilter_graph_config(filterGraph, nullptr);
//    if (ret < 0) {
//        printError(ret);
//        std::cerr << "Config FilterGraph Failed!" << std::endl;
//        return;
//    }
//}

//// 辅助函数：创建输入缓冲区滤镜
//void FFVFilter::createBufferFilter(AVFilterContext** ctx, AVCodecContext* codecCtx,
//                                   AVStream* stream, const char* name) {
//    const AVFilter* bufferFilter = avfilter_get_by_name("buffer");
//    if (!bufferFilter) {
//        std::cerr << "Buffer filter not found!" << std::endl;
//        return;
//    }

//    AVRational timeBase = stream->time_base;
//    char args[512];
//    snprintf(args, sizeof(args),
//             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
//             codecCtx->width, codecCtx->height,
//             AV_PIX_FMT_YUV420P,
//             timeBase.num, timeBase.den,
//             codecCtx->sample_aspect_ratio.num,
//             codecCtx->sample_aspect_ratio.den);

//    int ret = avfilter_graph_create_filter(ctx, bufferFilter, name, args, nullptr, filterGraph);
//    if (ret < 0) {
//        printError(ret);
//        std::cerr << "Create Buffer Filter Failed: " << name << std::endl;
//        return;
//    }
//}



//// 辅助函数：创建输出sink滤镜
//void FFVFilter::createBufferSinkFilter() {
//    const AVFilter* bufferSinkFilter = avfilter_get_by_name("buffersink");
//    if (!bufferSinkFilter) {
//        std::cerr << "Buffersink filter not found!" << std::endl;
//        return;
//    }

//    // 设置输出� �式
//    enum AVPixelFormat pixFmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

//    // 先分配上下文，再设置参数
//    int ret = avfilter_graph_create_filter(&bufferSinkCtx, bufferSinkFilter, "out", nullptr, nullptr, filterGraph);
//    if (ret < 0) {
//        printError(ret);
//        std::cerr << "Create BufferSink Failed!" << std::endl;
//        return;
//    }

////    ret = av_opt_set_int_list(bufferSinkCtx, "pix_fmts", pixFmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
////    if (ret < 0) {
////        printError(ret);
////        std::cerr << "Set BufferSink pixel formats failed!" << std::endl;
////    }
//}

//void FFVFilter::setOverlayPos(int x, int y, int w, int h)
//{
//    std::lock_guard<std::mutex> lock(filterMutex);
//    overlayX = x;
//    overlayY = y;
//    overlayW = w;
//    overlayH = h;
//}

void FFVFilter::sendEncodeFrame(AVFrame *frame)
{
    if(codecCtx1 == nullptr){
        codecCtx1 = vDecoder1->getCodecCtx();
        stream1 = vDecoder1->getStream();
    }

    encoderFrmQueue->enqueue(frame);
    av_frame_unref(frame);
    av_frame_free(&frame);
}

//void FFVFilter::getOverlayPos(int *x, int *y, int *w, int *h)
//{
//    std::lock_guard<std::mutex> lock(filterMutex);
//    *x = overlayX;
//    *y = overlayY;
//    *w = overlayW;
//    *h = overlayH;
//}

//void FFVFilter::linkFilters(AVFilterContext* src, int srcPad, AVFilterContext* dst, int dstPad) {
//    int ret = avfilter_link(src, srcPad, dst, dstPad);
//    if (ret < 0) {
//        printError(ret);
//        std::cerr << "Failed to link filters: " << avfilter_pad_get_name(src->output_pads, srcPad)
//                  << " -> " << avfilter_pad_get_name(dst->input_pads, dstPad) << std::endl;
//        return;
//    }
//}


//void FFVFilter::printError(int ret)
//{
//    char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
//    int res = av_strerror(ret,errorBuffer,sizeof errorBuffer);
//    if(res < 0){
//        std::cerr << "Unknow Error!" << std::endl;
//    }
//    else{
//        std::cerr << "Error:" << errorBuffer << std::endl;
//    }
//}
