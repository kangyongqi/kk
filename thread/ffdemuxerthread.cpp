#include "ffdemuxerthread.h"
#include"demuxer/ffdemuxer.h"
#include"queue/ffvpacketqueue.h"
#include"queue/ffapacketqueue.h"
#include"queue/ffeventqueue.h"
#include"player/ffplayercontext.h"
FFDemuxerThread::FFDemuxerThread()

{
    stopFlag.store(true);
}

FFDemuxerThread::~FFDemuxerThread()
{
    if(demuxer){
        delete demuxer;
        demuxer = nullptr;
    }
    if(playerCtx){
        delete playerCtx;
        playerCtx = nullptr;
    }
}


void FFDemuxerThread::init(FFDemuxer* demuxer_)
{
    demuxer = demuxer_;

    playerCtx = new FFPlayerContext();
    playerCtx->demuxerThread = this;
}

void FFDemuxerThread::wakeAllThread()
{
    if(demuxer){
        demuxer->wakeAllThread();
    }
    cond.notify_all();
}

void FFDemuxerThread::close()
{
    if(demuxer){
        demuxer->close();
    }
    cond.notify_all();
    stopFlag.store(true,std::memory_order_release);


}

bool FFDemuxerThread::peekStop()
{
    return stopFlag.load(std::memory_order_acquire);
}

void FFDemuxerThread::run()
{
    while(!m_stop){
        stopFlag.store(false,std::memory_order_release);
        int ret = demuxer->demux();
        //        std::cout << "stop = "<<m_stop <<std::endl;
        if(ret != 0){
           m_stop = true;
        }

        if(m_stop){
            break;
        }
    }

}

