#include "ffamuxerthread.h"
#include"muxer/ffmuxer.h"
#include"queue/ffapacketqueue.h"
#include"encoder/ffaencoder.h"
#include"queue/ffpacket.h"
FFAMuxerThread::FFAMuxerThread()
{

}

FFAMuxerThread::~FFAMuxerThread()
{

}

void FFAMuxerThread::init(FFAPacketQueue *pktQueue_, FFMuxer *muxer_, FFAEncoder *aEncoder_)
{
    pktQueue = pktQueue_;
    muxer = muxer_;
    aEncoder = aEncoder_;
}
void FFAMuxerThread::run()
{
    bool write[2] = {false,false};

    int sum = 0;
    while(!m_stop){
        FFPacket* pkt = pktQueue->dequeue();
        if(pkt == nullptr){
            continue;
        }
        AVPacket*packet = &pkt->packet;

        if(packet == nullptr){
            continue;
        }
        if(packet->data == 0){
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

        if(++sum == 5000){
            m_stop = true;
            std::cerr<<"Finish Audio!"<< std::endl;
        }
        std::cout<<"finish:"<<sum<<std::endl;
        if(ret < 0){
            m_stop = true;
            std::cerr<<"Finish Audio!"<< std::endl;
        }

        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    if(write[0] && write[1]){
        muxer->writeTrailer();
    }
}


