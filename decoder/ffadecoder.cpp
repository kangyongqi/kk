#include"ffadecoder.h"
#include"queue/ffaframequeue.h"
#include"resampler/ffaresampler.h"


FFADecoder::FFADecoder()
    :m_stop(false){
}

FFADecoder::~FFADecoder()
{
    close();
}

void FFADecoder::init(AVStream *stream_,FFAFrameQueue* frmQueue_)
{
    std::lock_guard<std::mutex> lock(mutex);
    m_stop = false;
    stream = stream_;
    frmQueue = frmQueue_;

    if( stream->codecpar == nullptr){
        return;
    }
   const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
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
//    av_dict_set(&codec_options, "fast", "1", 0);
    ret = avcodec_open2(codecCtx,codec,&codec_options);
    if(ret < 0)
    {
        printError(ret);
        avcodec_free_context(&codecCtx);
        return;
    }

}

void FFADecoder::flushDecoder()
{
    avcodec_flush_buffers(codecCtx);
}

FFAudioPars *FFADecoder::getAudioPars()
{
    return swraPars;
}

int FFADecoder::getToalSec()
{
    return static_cast<int>(0.5 + stream->duration * av_q2d(stream->time_base));
}

void FFADecoder::wakeAllThread()
{
    frmQueue->wakeAllThread();
}

void FFADecoder::stop()
{
    m_stop = true;
}

void FFADecoder::enqueueNull()
{
    frmQueue->enqueueNull();
}

void FFADecoder::flushQueue()
{
    frmQueue->flushQueue();
}

void FFADecoder::close()
{
    decode(nullptr);
    stop();
    std::lock_guard<std::mutex> lock(mutex);
    if(codecCtx){
        avcodec_free_context(&codecCtx);
    }
    if(aPars){
        delete  aPars;
        aPars = nullptr;
    }
    if(swraPars){
        delete swraPars;
        swraPars = nullptr;
    }
    if(resampler){
        delete resampler;
        resampler = nullptr;
    }
}

AVCodecContext *FFADecoder::getCodecCtx()
{
//    std::lock_guard<std::mutex> lock(mutex);
    return codecCtx;
}

AVStream* FFADecoder::getStream()
{
//    std::lock_guard<std::mutex> lock(mutex);
    return stream;
}

void FFADecoder::decode(AVPacket *packet)
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
            if(aPars == nullptr){
                aPars = new FFAudioPars();
                swraPars = new FFAudioPars();
                initAudioPars(frame);
                if(aPars->aFormatEnum != swraPars->aFormatEnum){
                    resampler = new FFAResampler();
                    initResampler();
                    printFmt();
                }
            }
            if(resampler){
                AVFrame* swrFrame = nullptr;
                resampler->resample(frame,&swrFrame);
                av_frame_unref(frame);
                if(m_stop.load(std::memory_order_acquire)){
                    av_frame_unref(swrFrame);
                    av_frame_free(&swrFrame);
                    m_stop.store(false,std::memory_order_release);
                    break;
                }
                else{
//                    std::cout<<"audio frame decode pts:"<< swrFrame->pts<<std::endl;
                    frmQueue->enqueue(swrFrame);
                    av_frame_unref(swrFrame);
                    av_frame_free(&swrFrame);
                }
            }
            else{
                if(m_stop.load(std::memory_order_acquire)){
                    av_frame_unref(frame);
                    m_stop.store(false,std::memory_order_release);
                    break;
                }
                else{
                    frmQueue->enqueue(frame);
                }
            }
        }
    }
    av_frame_free(&frame);

}

void FFADecoder::printError(int ret)
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

void FFADecoder::initAudioPars(AVFrame *frame)
{
    aPars->timeBase = stream->time_base;
    aPars->nbChannels = frame->ch_layout.nb_channels;
    aPars->aFormatEnum =codecCtx->sample_fmt;
    aPars->sampleSize = av_get_bytes_per_sample(codecCtx->sample_fmt);
    aPars->sampleRate = frame->sample_rate;

    memcpy(swraPars,aPars,sizeof(FFAudioPars));
    swraPars->aFormatEnum = AV_SAMPLE_FMT_FLTP;
    swraPars->sampleSize = av_get_bytes_per_sample(AV_SAMPLE_FMT_FLTP);
    swraPars->sampleRate = 48000;

}

void FFADecoder::initResampler()
{
    resampler->init(aPars,swraPars);
}

void FFADecoder::printFmt()
{
    std::cout << "audio format : "<< av_get_sample_fmt_name(aPars->aFormatEnum) << std::endl;
    std::cout << "sample_rate : "<< aPars->sampleRate << std::endl;
    std::cout << "channels : "<< aPars->nbChannels << std::endl;
    std::cout << "time_base : "<< aPars->timeBase.num << " / " << aPars->timeBase.den << std::endl;

    std::cout<<"================="<<std::endl;
    std::cout << "audio format : "<< av_get_sample_fmt_name(swraPars->aFormatEnum) << std::endl;
    std::cout << "sample_rate : "<< swraPars->sampleRate << std::endl;
    std::cout << "channels : "<< swraPars->nbChannels << std::endl;
    std::cout << "time_base : "<< swraPars->timeBase.num << " / " << swraPars->timeBase.den << std::endl;

}
