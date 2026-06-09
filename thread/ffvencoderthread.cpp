#include "ffvencoderthread.h"
#include"queue/ffvframequeue.h"
#include"queue/ffvpacketqueue.h"
#include"encoder/ffvencoder.h"
#include"muxer/ffmuxer.h"
#include"decoder/ffvdecoder.h"
#include"clock/ffglobalclock.h"
#include"filter/ffvfilter.h"
FFVEncoderThread::FFVEncoderThread()
{

}

FFVEncoderThread::~FFVEncoderThread()
{
}

void FFVEncoderThread::init(FFVFilter* vFilter_,FFVEncoder*vEncoder_,FFMuxer *muxer_,FFVFrameQueue *frmQueue_)
{
    vFilter = vFilter_;
    vEncoder = vEncoder_;
    muxer = muxer_;
    frmQueue = frmQueue_;
}

void FFVEncoderThread::wakeAllThread()
{
    if(frmQueue){
        frmQueue->wakeAllThread();
    }
    if(vEncoder){
        vEncoder->wakeAllThread();
    }
}

void FFVEncoderThread::close()
{
    if(vEncoder){
        vEncoder->close();
    }

    firstFrame = true;
    firstFramePts = 0;

    streamIndex = -1;

}

void FFVEncoderThread::run()
{

    while(!m_stop){

//        std::cout << "vEncoder Thread run " << std::endl;
        AVFrame* frame = frmQueue->dequeue();
        if(frame == nullptr){
            m_stop = true;
            break;
        }
        if(streamIndex == -1){
            initEncoder(frame);
        }

        if(firstFrame){
            firstFramePts = frame->pts;
            firstFrame = false;         
            vEncoder->encode(frame, streamIndex,0,timeBase);
        }
        else{
            int64_t relativePts = frame->pts - firstFramePts;
            vEncoder->encode(frame, streamIndex, relativePts,timeBase);
//            std::cerr << "relativePTS: " << relativePts << std::endl;
        }


        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}
void FFVEncoderThread::initEncoder(AVFrame*frame)
{
    frameRate = vFilter->getFrameRate();
//    frameRate = {30,1};
    timeBase = vFilter->getTimeBase();
//    timeBase ={1,10000000};

    vEncoder->initVideo(frame,frameRate);
    // 复用器根据这些参数创建视频流轨道，生成流头部信息（如MP4的 moov 盒子）
    muxer->addStream(vEncoder->getCodecCtx());
    // - 获取视频流在封装器中的唯一索引，后续编码的数据包需要携带这个索引，才能被写入正确的轨道
    streamIndex = muxer->getVStreamIndex();
}
