#include "ffmuxerthread.h"
#include"muxer/ffmuxer.h"
#include"queue/ffapacketqueue.h"
#include"queue/ffvpacketqueue.h"
#include"encoder/ffaencoder.h"
#include"encoder/ffvencoder.h"
#include"queue/ffpacket.h"
#include"event/ffcaptureprocessevent.h"
#include"queue/ffeventqueue.h"

#define CAPTURE_TIME 60

FFMuxerThread::FFMuxerThread()
{
    vTimeBase = {-1, -1};
    aTimeBase = {-1 ,-1};
}

FFMuxerThread::~FFMuxerThread()
{

}

void FFMuxerThread::init(FFAPacketQueue *aPktQueue_, FFVPacketQueue *vPktQueue_, FFMuxer *muxer_, FFAEncoder *aEncoder_, FFVEncoder *vEncoder_
                         ,FFCaptureContext* captureCtx_)
{
    aPktQueue = aPktQueue_;
    vPktQueue = vPktQueue_;

    aEncoder = aEncoder_;
    vEncoder = vEncoder_;

    muxer = muxer_;

    captureCtx = captureCtx_;
}

void FFMuxerThread::close()
{
    if(muxer){
        muxer->close();
    }

    aTimeBase = {-1 ,-1};
    vTimeBase = {-1, -1};

    aEncoder = nullptr;
    vEncoder = nullptr;

    lastProcessTime = 0;
}

void FFMuxerThread::wakeAllThread()
{
    if(vPktQueue){
        vPktQueue->wakeAllThread();
    }
    if(aPktQueue){
        aPktQueue->wakeAllThread();
    }
}

void FFMuxerThread::run() {
    bool audioFinish = true;
    bool videoFinish = true;

//    std::cout << "before m_stop = "<<m_stop << std::endl;
    muxer->writeHeader();
//    std::cout << "after m_stop = "<<m_stop << std::endl;

    FFPacket *audioPkt = nullptr;
    FFPacket *videoPkt = nullptr;
    AVPacket *aPacket = nullptr;
    AVPacket *vPacket = nullptr;

    double audioPtsSec = 0;// 存储音频帧的时间戳，以秒为单位
    double videoPtsSec = 0;// 存储视频帧的时间戳，以秒为单位

    int ret = 0;
    while (!m_stop) {
//        std::cout << "in muxer loop..." << std::endl;
        if(audioFinish){
            std::lock_guard<std::mutex> lock(mutex);
            audioPkt = aPktQueue->dequeue();
//            std::cout << "audioPkt dequeue" << std::endl;
            if(audioPkt == nullptr){
                m_stop = true;
                std::cerr << "audioPkt is nullptr" << std::endl;
                continue;
            }

            aPacket = &audioPkt->packet;
            if(aTimeBase.den == -1 && aTimeBase.num == -1){
                aTimeBase = aEncoder->getCodecCtx()->time_base;
            }
            audioPtsSec = aPacket->pts * av_q2d(aEncoder->getCodecCtx()->time_base);// av_q2d函数 将 AVRational 类型（有理数）转换为 double 类型（浮点数）
//            std::cout <<"audio pts:" <<aPacket->pts << std::endl;
            if(audioPtsSec < 0){
                audioFinish = true;
                av_packet_unref(aPacket);
                av_packet_free(&aPacket);
                continue;
            }
        }
        if(videoFinish){
            std::lock_guard<std::mutex> lock(mutex);
            videoPkt = vPktQueue->dequeue();
//             std::cout << "videoPkt dequeue" << std::endl;
            if(videoPkt == nullptr){
                std::cerr << "audioPkt is nullptr" << std::endl;
                m_stop = true;
                continue;
            }

            vPacket = &videoPkt->packet;

            if(vTimeBase.den == -1 && vTimeBase.num == -1){
                vTimeBase = vEncoder->getCodecCtx()->time_base;
//                std::cout << "vEncoder getCodecCtx" << std::endl;
            }
            videoPtsSec = vPacket->pts * av_q2d(vTimeBase);
            if(videoPtsSec < 0){
                videoFinish = true;
                av_packet_unref(vPacket);
                av_packet_free(&vPacket);
                continue;
            }
//            std::cout << "video pts: " <<vPacket->pts << std::endl;
        }


//        std::cout << "muxer thread stated !" << std::endl;
        if(audioPtsSec  < videoPtsSec){
//            std::cout<<"audio Finish:"<<audioPtsSec<<std::fixed<<std::endl;
            ret = muxer->mux(aPacket);
            if(ret < 0){
                std::cerr << "Mux Audio Fail !" << std::endl;
                m_stop = true;
                return;
            }
            audioFinish = true;
            videoFinish = false;

            sendCaptureProcessEvent(audioPtsSec);
        }
        else{
//            std::cout<<"video Finish:"<<videoPtsSec<<std::fixed<<std::endl;
            ret = muxer->mux(vPacket);
            if(ret < 0){
                std::cerr << "Mux Video Fail !" << std::endl;
                m_stop = true;
                return;
            }
//            std::cout<<"audio Finish:"<<audioPtsSec<<std::fixed<<std::endl;


            videoFinish = true;
            audioFinish = false;

            sendCaptureProcessEvent(videoPtsSec);

        }

        //        if(audioPtsSec >= CAPTURE_TIME && videoPtsSec >= CAPTURE_TIME){
        //            break;
        //        }
    }

    // 鍐欏叆鏂囦欢灏鹃儴
    muxer->writeTrailer();
}

void FFMuxerThread::sendCaptureProcessEvent(double seconds)
{
    if(seconds - lastProcessTime > 1.0){
        FFEvent* captureProcessEv = new FFCaptureProcessEvent(captureCtx,static_cast<int64_t>(seconds));
        FFEventQueue::getInstance().enqueue(captureProcessEv);

        lastProcessTime = seconds;
    }
}


