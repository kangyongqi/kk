#include"ffvdecoder.h"
#include"queue/ffvframequeue.h"
#include"resampler/ffvresampler.h"

FFVDecoder::FFVDecoder()
    :m_stop(false){

}

FFVDecoder::~FFVDecoder()
{
    close();
}

void FFVDecoder::init(AVStream *stream_,FFVFrameQueue* frmQueue_)
{
    std::lock_guard<std::mutex> lock(mutex);
    stream = stream_;
    frmQueue = frmQueue_;
    m_stop.store(false,std::memory_order_release);

    const AVCodec* codec = nullptr;
    if(stream->codecpar == nullptr){
        return;
    }

    if(stream->codecpar->codec_id == AV_CODEC_ID_H264){
        codec = avcodec_find_decoder_by_name("h264_nvdec");
        hardDecodeFlag = true;
//        codec = avcodec_find_decoder_by_name("h264");
//        hardDecodeFlag = false;
    }
    else{
        codec = avcodec_find_decoder(stream->codecpar->codec_id);
    }

    if(codec == nullptr){
        std::cerr<<"找不到视频解码器"<<std::endl;
        return;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if(codecCtx== nullptr){
        std::cerr<<"分配解码器上下文失败"<<std::endl;
        avcodec_free_context(&codecCtx);
        return;
    }

    int ret = avcodec_parameters_to_context(codecCtx,stream->codecpar);
    if(ret < 0)
    {
        printError(ret);
        avcodec_free_context(&codecCtx);
        return;
    }
    AVDictionary* codec_options = nullptr;
    if(!hardDecodeFlag){
        codecCtx->thread_count = av_cpu_count();
        av_dict_set(&codec_options, "fast", "1", 0);            // 启用快速解码模式

    }
    else{
        av_dict_set(&codec_options, "low_latency", "1", 0);
    }

    ret = avcodec_open2(codecCtx,codec,&codec_options);
    if(ret < 0)
    {
        printError(ret);
        avcodec_free_context(&codecCtx);
        return;
    }
    av_dict_free(&codec_options);



}

void FFVDecoder::flushDecoder()
{
    avcodec_flush_buffers(codecCtx);
}

int FFVDecoder::getTotalSec()
{
    return static_cast<int>(stream->duration * av_q2d(stream->time_base));
}

FFVideoPars* FFVDecoder::getVideoPars()
{
    return swsvPars;
}

AVCodecContext *FFVDecoder::getCodecCtx()
{
    //      std::lock_guard<std::mutex> lock(mutex);
    return codecCtx;
}

AVStream *FFVDecoder::getStream()
{
    //    std::lock_guard<std::mutex> lock(mutex);
    return stream;
}

void FFVDecoder::enqueueNull()
{
    frmQueue->enqueueNull();
}

void FFVDecoder::wakeAllThread()
{
    frmQueue->wakeAllThread();
}

void FFVDecoder::stop()
{
    m_stop.store(true,std::memory_order_release);
}

void FFVDecoder::flushQueue()
{
    frmQueue->flushQueue();
}

void FFVDecoder::close()
{
    decode(nullptr);
    stop();

    std::lock_guard<std::mutex> lock(mutex);
    if(codecCtx){
        avcodec_free_context(&codecCtx);
    }
    if(vPars){
        delete vPars;
        vPars = nullptr;
    }
    if(resampler){
        delete resampler;
        resampler = nullptr;
    }
    if(swsvPars){
        delete swsvPars;
        swsvPars = nullptr;
    }

    std::cout << "video Deocder close!" << std::endl;
}

void FFVDecoder::initResampler()
{
    resampler->init(vPars,swsvPars);
}

void FFVDecoder::initVideoPars(AVFrame *frame)
{
    vPars->timeBase = stream->time_base;
    vPars->pixFmtEnum = codecCtx->pix_fmt;
    vPars->frameRate = codecCtx->framerate;
    if(codecCtx->framerate.den == 0 || codecCtx->framerate.num == 0){
        vPars->frameRate = stream->avg_frame_rate;
        codecCtx->framerate = stream->avg_frame_rate; //调整frameRate
    }

    vPars->width = frame->width;
    vPars->height = frame->height;

    memcpy(swsvPars,vPars,sizeof(FFVideoPars));
    swsvPars->pixFmtEnum = AV_PIX_FMT_YUV420P;
}

void FFVDecoder::decode(AVPacket *packet)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(codecCtx == nullptr){
        return;
    }

    int ret = avcodec_send_packet(codecCtx,packet);
    if(ret < 0 && ret != AVERROR(EAGAIN)){
        printError(ret);
        avcodec_free_context(&codecCtx);
        return;
    }

    AVFrame* frame = av_frame_alloc();
    while(ret >= 0){
        if(m_stop.load(std::memory_order_acquire)){
            break;
        }

        ret = avcodec_receive_frame(codecCtx,frame);

        if(ret < 0){
            if(ret == AVERROR_EOF){
                break;
            }
            else if(ret == AVERROR(EAGAIN)){
                continue;
            }
            else{
                printError(ret);
                av_frame_free(&frame);
                avcodec_free_context(&codecCtx);
                return;
            }
        }
        else{
            if(vPars == nullptr){
                vPars = new FFVideoPars();
                swsvPars = new FFVideoPars();

                initVideoPars(frame);
                if(vPars->pixFmtEnum != swsvPars->pixFmtEnum){
                    resampler = new FFVResampler();
                    initResampler();
                }
            }

            if(resampler){
                AVFrame* swsFrame = nullptr;
                resampler->resample(frame,&swsFrame);
                av_frame_unref(frame);
                if(m_stop.load(std::memory_order_acquire)){
                    av_frame_unref(swsFrame);
                    av_frame_free(&swsFrame);
                    m_stop.store(false,std::memory_order_release);
                    break;
                }
                else{
                    //                    std::cout << "video frame pts: " <<swsFrame->pts<< std::endl;

                    //解码队列
                    AVFrame* decoderFrame = av_frame_clone(swsFrame);
                    //                    std::cout << "video frame pts: " <<decoderFrame->pts<< std::endl;
                    if(frmQueue !=nullptr){
                        frmQueue->enqueue(decoderFrame);
                    }
                    av_frame_unref(decoderFrame);
                    av_frame_free(&decoderFrame);

                    av_frame_unref(swsFrame);
                    av_frame_free(&swsFrame);
                }
            }
            else{
                if(m_stop.load(std::memory_order_acquire)){
                    av_frame_unref(frame);
                    break;
                }
                else{
                    //解码队列
                    if(frmQueue){
                        AVFrame* decoderFrame = av_frame_clone(frame);
                        frmQueue->enqueue(decoderFrame);
                        av_frame_free(&decoderFrame);
                        //                        std::cerr << "VIDEO decoder run !" << std::endl;
                    }

                    av_frame_unref(frame);
                    //                    std::cerr<<"decoder eunqueue!"<<std::endl;
                }
            }
        }
    }
    av_frame_free(&frame);
}

void FFVDecoder::printError(int ret)
{
    char errorBuffer[AV_ERROR_MAX_STRING_SIZE];
    int res = av_strerror(ret,errorBuffer,sizeof errorBuffer);
    if(res < 0){
        std::cerr << "Unknow Error!"<<std::endl;
    }
    else{
        std::cerr<<"Error:"<<errorBuffer<<std::endl;
    }

}
