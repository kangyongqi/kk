#include "ffendevent.h"



FFEndEvent::FFEndEvent(FFCaptureContext *captureContext, const std::string audioURL_[], const std::string videoURL_[], const std::string audioFMT_[], const std::string videoFMT_[], int audioTYPE_[], int videoTYPE_[])
    : FFEvent(captureContext)
{
    aDemuxerSize = FFCaptureContextType::A_DEMUXER_SIZE;
    vDemuxerSize = FFCaptureContextType::V_DEMUXER_SIZE;

    for(size_t i =0 ; i < aDemuxerSize; ++i){
        audioURL[i] = audioURL_[i];
        audioFMT[i] = audioFMT_[i];
        audioTYPE[i] = audioTYPE_[i];
        audioStartFlags[i] = false;

        aDemuxerThread[i] = captureContext->aDemuxerThread[i];
        aDecoderThread[i] = captureContext->aDecoderThread[i];
    }

    for(size_t i =0 ; i < vDemuxerSize; ++i){
        videoURL[i] = videoURL_[i];
        videoFMT[i] = videoFMT_[i];
        videoTYPE[i] = videoTYPE_[i];
        videoStartFlags[i] = false;

        vDemuxerThread[i] = captureContext->vDemuxerThread[i];
        vDecoderThread[i] = captureContext->vDecoderThread[i];
    }

    muxerThread = captureContext->muxerThread;

    aEncoderThread = captureContext->aEncoderThread;
    vEncoderThread = captureContext->vEncoderThread;

    aFilterThread = captureContext->aFilterThread;
    vFilterThread = captureContext->vFilterThread;
}
FFEndEvent::~FFEndEvent()
{

}

void FFEndEvent::work()
{

}

void FFEndEvent::startAll()
{
    //启动音频线程
    for(size_t i =0 ; i < aDemuxerSize; ++i){
        if(audioURL[i].empty() || audioFMT[i].empty() || audioTYPE[i] == FFCaptureContextType::NOTYPE){
            continue;
        }
        else{
            aDemuxerThread[i]->start();
            aDecoderThread[i]->start();
            audioStartFlags[i] = true;
        }
    }


    //启动视频线程
    for(size_t i =0 ; i < aDemuxerSize; ++i){
        if(videoURL[i].empty() || videoFMT[i].empty() || videoTYPE[i] == FFCaptureContextType::NOTYPE){
            continue;
        }
        else{
            vDemuxerThread[i]->start();
            vDecoderThread[i]->start();
            videoStartFlags[i] = true;
        }
    }

    //启动过滤线程
    aFilterThread->start();
    vFilterThread->start();

    //启动编码线程
    aEncoderThread->start();
    vEncoderThread->start();

    //启动复用线程
    muxerThread->start();
}

void FFEndEvent::stopAll()
{

}
