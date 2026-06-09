#include "ffvdecoderthread.h"
#include"queue/ffvpacketqueue.h"
#include"decoder/ffvdecoder.h"
#include"queue/ffpacket.h"
#include"queue/ffeventqueue.h"
#include"player/ffplayercontext.h"

FFVDecoderThread::FFVDecoderThread()
{
    stopFlag = true;
}

FFVDecoderThread::~FFVDecoderThread()
{
    if(vPktQueue){
        delete vPktQueue;
        vPktQueue = nullptr;
    }
    if(vDecoder){
        delete vDecoder;
        vDecoder = nullptr;
    }
    if(playerCtx){
        delete playerCtx;
        playerCtx = nullptr;
    }
}

void FFVDecoderThread:: init(FFVDecoder *vDecoder_, FFVPacketQueue* vPktQueue_)
{
    vDecoder = vDecoder_;
    vPktQueue = vPktQueue_;

    playerCtx = new FFPlayerContext();
    playerCtx->vDecoderThread = this;
    playerCtx->vPktQueue = vPktQueue_;
}

void FFVDecoderThread::wakeAllThread()
{
    if(vPktQueue){
        vPktQueue->wakeAllThread();
    }
    if(vDecoder){
        vDecoder->wakeAllThread();
    }
}

void FFVDecoderThread::close()
{
    if(vDecoder){
        vDecoder->close();
    }
    stopFlag.store(true,std::memory_order_release);
}

bool FFVDecoderThread::peekStop()
{
    return stopFlag.load(std::memory_order_acquire);
}

void FFVDecoderThread::run()
{
    while(!m_stop){
          stopFlag.store(false,std::memory_order_release);
        FFPacket* pkt = vPktQueue->dequeue();
        if(pkt == nullptr){
            continue;
        }

        if(pkt->serial != vPktQueue->getSerial()){ //seek操作
            vPktQueue->flushQueue();
            vDecoder->flushQueue();
            vDecoder->flushDecoder();


            std::cerr<<"flush vDecoder"<<std::endl;
        }
        else{
            if(pkt->type == NULLP && pkt->packet.data == nullptr){ //读取完毕，冲刷解码器
//                m_stop = true;
                vDecoder->decode(nullptr);
                std::cout<<"null vpacket"<<std::endl;
//                sendEndEvent();
                vDecoder->enqueueNull();
            }
            else{ //正常读取
//                auto start = av_gettime_relative();

                vDecoder->decode(&pkt->packet);
//                auto end = av_gettime_relative();

//                std::cerr << "decode cost :" << (end - start ) / 1000  << std::endl;
            }
            av_packet_unref(&pkt->packet);
            av_freep(&pkt);
        }
    }

}

