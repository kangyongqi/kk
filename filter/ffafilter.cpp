#include "ffafilter.h"
#include "queue/ffaframequeue.h"
#include "decoder/ffadecoder.h"
#include"clock/ffglobalclock.h"
#include"capture/ffcapturecontext.h"

using namespace  FFCaptureContextType;

#define FF_AUDIO_TIME_BASE {1 , 48000 }

FFAFilter::FFAFilter()
{

}

FFAFilter::~FFAFilter()
{
    if (filterGraph) {
        avfilter_graph_free(&filterGraph);
    }
    if(singleFilterGraph[0]){
        avfilter_graph_free(&singleFilterGraph[0]);
    }
    if(singleFilterGraph[1]){
        avfilter_graph_free(&singleFilterGraph[1]);
    }
}


void FFAFilter::init(FFAFrameQueue *encoderFrmQueue_, FFADecoder *aDecoder1_, FFADecoder *aDecoder2_)
{
    encoderFrmQueue = encoderFrmQueue_;

    aDecoder1 = aDecoder1_;
    aDecoder2 = aDecoder2_;
}

AVRational FFAFilter::getTimeBase()
{
    return FF_AUDIO_TIME_BASE;
}

AVMediaType FFAFilter::getMediaType()
{
    return AVMEDIA_TYPE_AUDIO;
}

void FFAFilter::setAudioVolume(double value) {
    std::lock_guard<std::mutex> lock(mutex);
    audioVolume = value;

    if(singleFilterGraph[0]){
        std::cout << "adjust audio volume:" << audioVolume << std::endl;
        destroyFilterGraph(singleFilterGraph[0]);
        initSingleFilter(aDecoder1->getCodecCtx(),aDecoder1->getStream(),AUDIO);
    }
}

void FFAFilter::destroyFilterGraph(AVFilterGraph* filterGraph) {
    if (filterGraph) {
        avfilter_graph_free(&filterGraph);
        filterGraph = nullptr;
    }
}

void FFAFilter::setMicrophoneVolume(double value)
{
    std::lock_guard<std::mutex>lock(mutex);
    microphoneVolume = value;
    if (singleVolumeCtx[1]) {
        std::cout << "adjust microphone volume:" << microphoneVolume << std::endl;
        destroyFilterGraph(singleFilterGraph[1]);
        initSingleFilter(aDecoder2->getCodecCtx(),aDecoder2->getStream(),MICROPHONE);
    }
}

void FFAFilter::setVolume(double value1, double value2)
{
    std::lock_guard<std::mutex>lock(mutex);
    if(value1 != -1){
        audioVolume = value1;
    }
    if(value2 !=-1){
        microphoneVolume = value2;
    }
    if(filterGraph){
        std::cout << "adjust audio volume:" << audioVolume << std::endl;
        std::cout << "adjust microphone volume:" << microphoneVolume << std::endl;

        destroyFilterGraph(filterGraph);
        initFilter(aDecoder1->getCodecCtx(),aDecoder2->getCodecCtx(),aDecoder1->getStream(),aDecoder2->getStream());
    }
}

// 发送音频帧到过滤器（双路混音）
int FFAFilter::sendFilter(AVFrame *frame1, AVFrame *frame2,int64_t start,int64_t pauseTime)
{
    std::lock_guard<std::mutex>lock(mutex);
    if(!frame1 || !frame2){
        if(frame1){
            av_frame_unref(frame1);
            av_frame_free(&frame1);
        }
        if(frame2){
            av_frame_unref(frame2);
            av_frame_free(&frame2);
        }
        return 0;
    }

    if (!bufferCtx1 || !bufferCtx2) {
        initFilter(aDecoder1->getCodecCtx(), aDecoder2->getCodecCtx(),
                   aDecoder1->getStream(), aDecoder2->getStream());
    }

    // 发送音频帧到滤镜图
    int ret = av_buffersrc_add_frame_flags(bufferCtx1, frame1, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        std::cerr << "Failed to send frame to buffer1" << std::endl;
        printError(ret);
        return -1;
    }

    ret = av_buffersrc_add_frame_flags(bufferCtx2, frame2, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        std::cerr << "Failed to send frame to buffer2" << std::endl;
        printError(ret);
        return -1;
    }

    // 循环读取所有处理后的帧
    while (true) {
        AVFrame* filterFrame = av_frame_alloc();
        // 成品出口
        ret = av_buffersink_get_frame(bufferSinkCtx, filterFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&filterFrame);
            break;
        }
        if (ret < 0) {
            std::cerr << "Failed to get frame from sink" << std::endl;
            printError(ret);
            av_frame_free(&filterFrame);
            return -1;
        }

        encoderFrmQueue->enqueue(filterFrame);

        //调整时间戳
        auto end = av_gettime_relative();
        int64_t duration = 10 * (end - start);
        filterFrame->pts = start + duration - pauseTime * 10;// av_gettime_relative() * 10

        av_frame_unref(filterFrame);// 释放音频帧内部的数据（比如音频缓冲区）
        av_frame_free(&filterFrame);// 释放整个音频帧对象（包括内部数据和帧结构体本身）
    }


    av_frame_unref(frame1);
    av_frame_free(&frame1);

    av_frame_unref(frame2);
    av_frame_free(&frame2);

    return 0;
}
// 直接发送音频帧到编码器队列（无过滤）
void FFAFilter::sendFrame(AVFrame *frame,int64_t start,int64_t pauseTime)
{
    if(frame == nullptr){
        return;
    }
    auto end = av_gettime_relative() * 10;
    auto duration = (end - start);
    frame->pts = av_gettime_relative() * 10 - duration - pauseTime * 10;
    encoderFrmQueue->enqueue(frame);
    av_frame_free(&frame);
}

void FFAFilter::initFilter(AVCodecContext *codecCtx1_, AVCodecContext *codecCtx2_,
                           AVStream *stream1_, AVStream *stream2_)
{
    codecCtx1 = codecCtx1_;
    codecCtx2 = codecCtx2_;
    stream1 = stream1_;
    stream2 = stream2_;

    filterGraph = avfilter_graph_alloc();
    if (!filterGraph) {
        throw std::runtime_error("Failed to allocate filter graph");
    }

    // 创建输入缓冲区滤镜
    createBufferFilter(filterGraph,&bufferCtx1, codecCtx1, stream1, "in1");
    createBufferFilter(filterGraph,&bufferCtx2, codecCtx2, stream2, "in2");

    //创建音量滤镜
    createVolumeFilter(filterGraph, &volumeCtx1,audioVolume,"volume1");
    createVolumeFilter(filterGraph, &volumeCtx2,microphoneVolume,"volume2");

    // 创建混音滤镜
    createAmixFilter(filterGraph);

    // 创建降噪滤镜
    //    createAfftdnFilter(filterGraph);

    // 创建输出sink滤镜
    createBufferSinkFilter(filterGraph,&bufferSinkCtx,"out");

    // 构建滤镜链
    linkFilters(bufferCtx1, 0, volumeCtx1, 0);    // 输入1 -> 音量
    linkFilters(volumeCtx1,0,amixCtx,0);          //音量 -> 混音

    linkFilters(bufferCtx2, 0, volumeCtx2, 0);    // 输入2 -> 音量
    linkFilters(volumeCtx2, 0, amixCtx, 1);    // 音量 -> 混音

    //    linkFilters(amixCtx, 0, afftdnCtx, 0);     // 混音 -> 降噪
    linkFilters(amixCtx, 0, bufferSinkCtx, 0); // 降噪 -> 输出

    // 配置滤镜图
    // 设置滤镜图是否自动插入格式转换滤镜（如格式、采样率、声道数转换），解决不同滤镜之间的格式不兼容问题。
    avfilter_graph_set_auto_convert(filterGraph, AVFILTER_AUTO_CONVERT_ALL);// 格式自动转换
    // 验证并初始化整个滤镜图的所有滤镜上下文，建立滤镜之间的数据通路
    int ret = avfilter_graph_config(filterGraph, nullptr);// 验证滤镜图及上述配置是否安装成功
    if (ret < 0) {
        printError(ret);
        throw std::runtime_error("Filter graph config failed");
    }
}

// 辅助函数：创建输入缓冲区滤镜
void FFAFilter::createBufferFilter(AVFilterGraph* filterGraph,AVFilterContext** ctx,
                                   AVCodecContext* codecCtx,
                                   AVStream* stream,
                                   const char* name)
{
    // 你要建一个投料口，先去仓库找“abuffer”这个型号的设备图纸。这是专门用来接收原始音频帧的 “投料口” 工具
    const AVFilter* bufferFilter = avfilter_get_by_name("abuffer");
    if (!bufferFilter) {
        throw std::runtime_error("abuffer filter not found");
    }
    // 音频不是随便扔进去就行的，得提前说清楚四个关键信息：
    // 获取时间基
    AVRational timeBase1 = stream->time_base;// 时间单位（timeBase1）：音频的"年龄"用什么单位计算
    AVChannelLayout layout = {};// 通道布局（layout）：音频的"声音方向"是什么样的
    uint64_t channel_mask = 0;// 通道掩码（channel_mask）：用数字表示的"声音方向"

    // 先按音频的通道数，比如 2 声道 = 立体声。生成默认布局，再把布局转换成机器能认的channel_mask
    av_channel_layout_default(&layout,codecCtx->ch_layout.nb_channels);// 根据声道数自动选择合适的通道布局
    if (layout.order == AV_CHANNEL_ORDER_NATIVE) {
        channel_mask = layout.u.mask;
    }

    std::ostringstream args;
    args << "time_base=" << timeBase1.num << "/" << timeBase1.den
         << ":sample_rate=" << FF_SAMPLE_RATE
         << ":sample_fmt=" << av_get_sample_fmt_name(FF_SAMPLE_FMT)
         << ":channel_layout=0x" << std::hex << channel_mask;
    // 装上这个配置好的投料口。装好了之后，你就可以通过 ctx 这个“投料口把手”，往里面塞音频帧了。
    // 拿着 “投料口工具”+“使用说明书”，把这个投料口装到生产线（filterGraph）上，并且给投料口贴个名字（name）
    int ret = avfilter_graph_create_filter(ctx, bufferFilter, name, args.str().c_str(),
                                           nullptr, filterGraph);
    if (ret < 0) {
        printError(ret);
        throw std::runtime_error("Failed to create abuffer filter");
    }
}

// 创建混音滤镜
void FFAFilter::createAmixFilter(AVFilterGraph* filterGraph)
{
    const AVFilter* amixFilter = avfilter_get_by_name("amix");
    amixCtx = createFilter(filterGraph,amixFilter, "amix", "inputs=2:duration=longest");
}

// 创建降噪滤镜
void FFAFilter::createAfftdnFilter(AVFilterGraph* filterGraph)
{
    const AVFilter* afftdnFilter = avfilter_get_by_name("afftdn");
    afftdnCtx = createFilter(filterGraph,afftdnFilter, "afftdn", "nf=-60:track_noise=1");
}

// 创建输出sink滤镜
void FFAFilter::createBufferSinkFilter(AVFilterGraph* filterGraph,AVFilterContext **ctx,const char *name)
{
    const AVFilter* sinkFilter = avfilter_get_by_name("abuffersink");
    *ctx = createFilter(filterGraph,sinkFilter, name, nullptr);

    // 设置输出格式
    enum AVSampleFormat fmts[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
    av_opt_set_int_list(bufferSinkCtx, "sample_fmts", fmts,
                        AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
}

// 创建音量滤镜
void FFAFilter::createVolumeFilter(AVFilterGraph* filterGraph,AVFilterContext** ctx, double gain, const char* name)
{
    const AVFilter* volumeFilter = avfilter_get_by_name("volume");
    if (!volumeFilter) {
        throw std::runtime_error("volume filter not found");
    }

    char args[64];

    // 比如限定了 64 个字符的长度，避免参数过长导致缓冲区溢出，相当于给说明书做了 “格式校验”，防止内容超页、乱码
    snprintf(args, sizeof(args), "volume=%.2f:eval=frame", gain);

    int ret = avfilter_graph_create_filter(ctx, volumeFilter, name, args, nullptr, filterGraph);
    if (ret < 0) {
        printError(ret);
        throw std::runtime_error("Failed to create volume filter: " + std::string(name));
    }
}

void FFAFilter:: initSingleFilter(AVCodecContext* codecCtx,AVStream* stream,int type)
{
    size_t index = (type == AUDIO) ? 0 : 1;

    singleFilterGraph[index] = avfilter_graph_alloc();
    if (!singleFilterGraph[index]) {
        throw std::runtime_error("Failed to allocate filter graph");
    }

    // 创建输入缓冲区滤镜
    const std::string bufferFilterName = "single_in" + std::to_string(index);
    createBufferFilter(singleFilterGraph[index], &singleBufferCtx[index], codecCtx, stream, bufferFilterName.c_str());

    //创建音量滤镜
    const std::string volumeFilterName = "single_volume" + std::to_string(index);
    double volume = (type == AUDIO) ? audioVolume : microphoneVolume;
    createVolumeFilter(singleFilterGraph[index], &singleVolumeCtx[index],volume ,volumeFilterName.c_str());

    // 创建输出sink滤镜
    const std::string sinkFilterName = "single_out" + std::to_string(index);
    createBufferSinkFilter(singleFilterGraph[index],&singleBufferSinkCtx[index],sinkFilterName.c_str());

    // 构建滤镜链
    linkFilters(singleBufferCtx[index], 0, singleVolumeCtx[index], 0);
    linkFilters(singleVolumeCtx[index],0,singleBufferSinkCtx[index],0);

    // 配置滤镜图
    avfilter_graph_set_auto_convert(singleFilterGraph[index], AVFILTER_AUTO_CONVERT_ALL);
    int ret = avfilter_graph_config(singleFilterGraph[index], nullptr);

    if (ret < 0) {
        printError(ret);
        throw std::runtime_error("Filter graph config failed");
    }
}

void FFAFilter::sendSingleFilter(AVFrame *frame, int64_t start,int64_t pauseTime,int type)
{
    std::lock_guard<std::mutex>lock(mutex);
    if(frame == nullptr){
        return;
    }

    size_t index = (type == AUDIO) ? 0 : 1;
    if(!singleFilterGraph[index]){
        if(index == 0){
            initSingleFilter(aDecoder1->getCodecCtx(),aDecoder1->getStream(),AUDIO);
        }
        else{
            initSingleFilter(aDecoder2->getCodecCtx(),aDecoder2->getStream(),MICROPHONE);
        }
    }


    // 发送音频帧到滤镜图
    // 它是滤镜链的 “数据出口”—— 所有流经滤镜链（比如你之前的 volume 滤镜）处理后的音频帧，最终都会通过这个函数被读取出来，供后续的编码、播放或保存使用。
    int ret = av_buffersrc_add_frame_flags(singleBufferCtx[index], frame, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret < 0) {
        std::cerr << "Failed to send frame to singleBufferCtx" << std::endl;
        printError(ret);
        return;
    }

    // 循环读取所有处理后的帧
    while (true) {
        AVFrame* filterFrame = av_frame_alloc();
        ret = av_buffersink_get_frame(singleBufferSinkCtx[index], filterFrame);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_frame_free(&filterFrame);
            break;
        }
        if (ret < 0) {
            std::cerr << "Failed to get frame from sink" << std::endl;
            printError(ret);
            av_frame_free(&filterFrame);
        }

        encoderFrmQueue->enqueue(filterFrame);

        //调整时间戳
        auto end = av_gettime_relative();
        int64_t duration = 10 * (end - start);
        filterFrame->pts = av_gettime_relative() + duration - pauseTime * 10;

        av_frame_unref(filterFrame);
        av_frame_free(&filterFrame);
    }

    av_frame_unref(frame);
    av_frame_free(&frame);
}


// 通用滤镜创建函数
AVFilterContext* FFAFilter::createFilter(AVFilterGraph* filterGraph,const AVFilter* filter,
                                         const char* name,
                                         const char* filterArgs)
{
    AVFilterContext* ctx = nullptr;
    int ret = avfilter_graph_create_filter(&ctx, filter, name, filterArgs,
                                           nullptr, filterGraph);
    if (ret < 0) {
        printError(ret);
        throw std::runtime_error(std::string("Failed to create ") + name);
    }
    return ctx;
}

// 安全链接滤镜
void FFAFilter::linkFilters(AVFilterContext* src, int srcPad,
                            AVFilterContext* dst, int dstPad)
{
    int ret = avfilter_link(src, srcPad, dst, dstPad);
    if (ret < 0) {
        printError(ret);
        throw std::runtime_error("Filter linking failed");
    }
}


void FFAFilter::printError(int ret)
{
    char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
    int res = av_strerror(ret, errorBuffer, sizeof errorBuffer);
    if (res < 0) {
        std::cerr << "Unknow Error!" << std::endl;
    }
    else {
        std::cerr << "Error:" << errorBuffer << std::endl;
    }
}
