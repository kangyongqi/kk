#include "ffvmuxerthread.h"
#include"muxer/ffmuxer.h"
#include"queue/ffvpacketqueue.h"
#include"encoder/ffvencoder.h"
#include"queue/ffpacket.h"

FFVMuxerThread::FFVMuxerThread()
{

}

FFVMuxerThread::~FFVMuxerThread()
{

}

void FFVMuxerThread::init(FFVPacketQueue *pktQueue_, FFMuxer *muxer_)
{
    pktQueue = pktQueue_;
    muxer = muxer_;
}

void FFVMuxerThread::run()
{
    bool write[2] = {false,false};
    int sum = 0; //测试
    while(!m_stop){
        FFPacket* pkt = pktQueue->dequeue();
        if(pkt == nullptr){
            continue;
        }
        AVPacket*packet = &pkt->packet;

        if(packet == nullptr){
            continue;
        }
        if(packet->data == nullptr){
            av_packet_unref(packet);
            av_packet_free(&packet);
            continue;
        }

        if(!write[0]){
            muxer->writeHeader();
            write[0] = true;
        }

        int ret = muxer->mux(packet);
        if(!write[1]){
            write[1] = true;
        }

        if(ret < 0){
            m_stop = true;
        }


        if(++sum == 500){
            m_stop = true;
            std::cerr<<"Finished Video!" << std::endl;
        }
        std::cout<<"finish:"<<sum<<std::endl;
        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    if(write[0] && write[1]){
        muxer->writeTrailer();
    }
}
