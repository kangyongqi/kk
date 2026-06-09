#include "ffadecoderthread.h"
#include"queue/ffapacketqueue.h"
#include"decoder/ffadecoder.h"
#include"queue/ffpacket.h"
#include"player/ffplayercontext.h"
#include"event/ffendevent.h"
#include"queue/ffeventqueue.h"
FFADecoderThread::FFADecoderThread()
{
    stopFlag = true;
}

FFADecoderThread::~FFADecoderThread()
{
    if(aPktQueue){
        delete aPktQueue;
        aPktQueue = nullptr;
    }
    if(aDecoder) {
        delete aDecoder;
        aDecoder = nullptr;
    }
    if(playerCtx){
        delete playerCtx;
        playerCtx = nullptr;
    }

}

void FFADecoderThread::init(FFADecoder *aDecoder_, FFAPacketQueue* aPktQueue_)
{
    aDecoder = aDecoder_;
    aPktQueue = aPktQueue_;

    playerCtx = new FFPlayerContext();
    playerCtx->aDecoderThread = this;
    playerCtx->aPktQueue = aPktQueue_;

}

void FFADecoderThread::wakeAllThread()
{
    if(aPktQueue){
        aPktQueue->wakeAllThread();
    }
    if(aDecoder){
        aDecoder->wakeAllThread();
    }
}

void FFADecoderThread::close()
{
    if(aDecoder){
        aDecoder->close();
    }
    stopFlag.store(true,std::memory_order_release);
}

bool FFADecoderThread::peekStop()
{
    return stopFlag.load(std::memory_order_acquire); //提供查询接口，让其他线程检查停止状态。返回true/false，不执行任何操作
}

void FFADecoderThread::run()
{
    while(!m_stop){
        stopFlag.store(false,std::memory_order_release);
        FFPacket* pkt = aPktQueue->dequeue();
        if(pkt == nullptr){
            continue;
        }
        if(pkt->serial != aPktQueue->getSerial()){ //seek鎿嶄綔
            aPktQueue->flushQueue();
            aDecoder->flushQueue();

            aDecoder->flushDecoder();

            std::cerr<<"flush aDecoder"<<std::endl;
        }
        else{
            if(pkt->type == NULLP && pkt->packet.data == nullptr){ //璇诲彇瀹屾瘯锛屽啿鍒疯В鐮佸櫒
//                m_stop = true;
                aDecoder->decode(nullptr);
                std::cerr<< "null apacket !"<< std::endl;
//                sendEndEvent();
                aDecoder->enqueueNull();
            }
            else{ //姝ｅ父璇诲彇
                aDecoder->decode(&pkt->packet);

            }
            av_packet_unref(&pkt->packet);
            av_freep(&pkt);// 通用的释放函数，万能垃圾桶，其他的地方有专用的函数，所以不用他
        }
    }
}


