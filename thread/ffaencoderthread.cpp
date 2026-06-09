#include "ffaencoderthread.h"
#include"queue/ffaframequeue.h"
#include"queue/ffapacketqueue.h"
#include"encoder/ffaencoder.h"
#include"muxer/ffmuxer.h"
#include"decoder/ffadecoder.h"
#include"filter/ffafilter.h"

FFAEncoderThread::FFAEncoderThread()
{

}

FFAEncoderThread::~FFAEncoderThread()
{

}

void FFAEncoderThread::init(FFAFilter* aFilter_,FFAEncoder *aEncoder_, FFMuxer *muxer_, FFAFrameQueue *frmQueue_)
{
    aEncoder = aEncoder_;
    muxer = muxer_;
    frmQueue = frmQueue_;
    aFilter = aFilter_;
}

void FFAEncoderThread::close()
{
    if(aEncoder){
        aEncoder->close();
    }

    firstFrame = true;    // 标记是否为第一帧
     firstFramePts = 0;  // 用于存储第一帧的绝对PTS

     streamIndex = -1;
}

void FFAEncoderThread::wakeAllThread()
{
    if(frmQueue){
        frmQueue->wakeAllThread();
    }
    if(aEncoder){
        aEncoder->wakeAllThread();
    }

}

void FFAEncoderThread::run()
{


    while(!m_stop){
        AVFrame* frame = frmQueue->dequeue();
//        std::cerr << "aEncoderFrmQueue dequeue !" << std::endl;
        if(frame == nullptr){
            m_stop = true;
            break;
        }
        if(streamIndex == -1){
            initEncoder(frame);
        }

        if(firstFrame){
            firstFramePts = frame->pts;  // 记录第一帧的绝对PTS
            firstFrame = false;

            // 第一帧的相对PTS为0
            aEncoder->encode(frame, streamIndex, 0, audioTimeBase);
        }
        else{
            // 计算当前帧的相对PTS
            int64_t relativePts = frame->pts - firstFramePts;
            aEncoder->encode(frame, streamIndex, relativePts,  audioTimeBase);
//            std::cerr << "relativePTS: " << relativePts<< std::endl;
        }


        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

void FFAEncoderThread::initEncoder(AVFrame* frame)
{
    audioTimeBase = aFilter->getTimeBase();
    aEncoder->initAudio(frame);

    muxer->addStream(aEncoder->getCodecCtx());
    streamIndex = muxer->getAStreamIndex();
}
