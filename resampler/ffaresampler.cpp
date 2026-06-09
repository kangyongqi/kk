#include "ffaresampler.h"
#include "decoder/ffadecoder.h"

FFAResampler::FFAResampler()
{

}

FFAResampler::~FFAResampler()
{
    if (swrCtx) {
        swr_free(&swrCtx);
    }
    if (srcPars) {
        delete srcPars;
        srcPars = nullptr;
    }
    if (dstPars) {
        delete dstPars;
        dstPars = nullptr;
    }
}

void FFAResampler::init(FFAudioPars *src_, FFAudioPars *dst_)
{
    srcPars = new FFAudioPars();
    // 这个复制是搬箱子，不像copyto只是复制文件
    memcpy(srcPars, src_, sizeof(FFAudioPars));

    dstPars = new FFAudioPars();
    memcpy(dstPars, dst_, sizeof(FFAudioPars));

    initSwr();
}

void FFAResampler::resample(AVFrame *srcFrame, AVFrame **dstFrame) {
    // 获取重采样延迟
    int64_t delaySamples = swr_get_delay(swrCtx, srcPars->sampleRate);
    // 计算最大输出样本数
    int maxNbSamples = swr_get_out_samples(swrCtx, srcFrame->nb_samples + delaySamples);

    *dstFrame = allocFrame(dstPars, maxNbSamples, srcFrame);
    if (!*dstFrame) {
        std::cerr << "av_frame_alloc error!" << std::endl;
        return;
    }

    // 调整输出帧的 PTS
    (*dstFrame)->pts = av_rescale_q(
                srcFrame->pts + delaySamples,
                (AVRational){1, srcPars->sampleRate},
                (AVRational){1, dstPars->sampleRate}
                );
//    std::cout << "srcFrame->pts:" <<srcFrame->pts <<  std::endl;
//    std::cout << "dstFrame->pts:" << (*dstFrame)->pts <<std::endl;

    int samples = swr_convert(
                swrCtx,
                (*dstFrame)->data,
                maxNbSamples,
                (const uint8_t **)srcFrame->data,
                srcFrame->nb_samples
                );
    if (samples < 0) {
        printError(samples);
        swr_free(&swrCtx);
        return;
    }

    // 更新输出帧的实际样本数
    (*dstFrame)->nb_samples = samples;
}

void FFAResampler::initSwr()
{
#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(57, 100, 0)
    // 旧版本使用av_get_default_channel_layout
    swrCtx = swr_alloc_set_opts(swrCtx,
                                av_get_default_channel_layout(dstPars->nbChannels), dstPars->aFormatEnum, dstPars->sampleRate,
                                av_get_default_channel_layout(srcPars->nbChannels), srcPars->aFormatEnum, srcPars->sampleRate,
                                0, nullptr);
#else
    // 新版本使用AVChannelLayout和swr_alloc_set_opts2


    // 初始化默认通道布局
    av_channel_layout_default(&srcLayout, srcPars->nbChannels);
    av_channel_layout_default(&dstLayout, dstPars->nbChannels);

    // 创建并配置SwrContext
    int ret = swr_alloc_set_opts2(&swrCtx,
                                  &dstLayout, dstPars->aFormatEnum, dstPars->sampleRate,
                                  &srcLayout, srcPars->aFormatEnum, srcPars->sampleRate,
                                  0, nullptr);
    if (ret < 0) {
        std::cerr << "Swr Alloc Set Opts Fail !" << std::endl;
        printError(ret);
        swrCtx = nullptr;
    }

    // 清理通道布局资源
    av_channel_layout_uninit(&srcLayout);
    av_channel_layout_uninit(&dstLayout);
#endif
    if (!swrCtx) {
        std::cerr << "initSwr error!" << std::endl;
        return;
    }

    ret = swr_init(swrCtx);
    if (ret < 0) {
        printError(ret);
        swr_free(&swrCtx);
        return;
    }
}

AVFrame* FFAResampler::allocFrame(FFAudioPars* aPars, int nbSamples, AVFrame* srcFrame)
{
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return nullptr;
    }

    frame->format = aPars->aFormatEnum;
    frame->sample_rate = aPars->sampleRate;
    frame->nb_samples = nbSamples;

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 100, 0)
    // FFmpeg 7.0+ 新API
    av_channel_layout_default(&frame->ch_layout, aPars->nbChannels);
#else
    // 旧版本兼容
    frame->channels = aPars->nbChannels;
    frame->channel_layout = av_get_default_channel_layout(aPars->nbChannels);
#endif

    if (srcFrame) {
        frame->pts = srcFrame->pts;
    }

    int ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        printError(ret);
        av_frame_unref(frame);
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}
void FFAResampler::printError(int ret)
{
    char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
    int res = av_strerror(ret, errorBuffer, sizeof(errorBuffer));
    if (res < 0) {
        std::cerr << "Unknown Error!" << std::endl;
    } else {
        std::cerr << "Error: " << errorBuffer << std::endl;
    }
}
