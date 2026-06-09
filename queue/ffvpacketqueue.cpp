#include "ffvpacketqueue.h"
#include"ffpacket.h"

#define MAX_PACKET_SIZE 2

FFVPacketQueue::FFVPacketQueue()
    : serial(0),m_stop(false)
{
}

FFVPacketQueue::~FFVPacketQueue()
{
    close();
}

FFPacket* FFVPacketQueue::dequeue()
{
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock,[this]() { return !pktQueue.empty() || m_stop.load(); });
    if(m_stop.load()){
        return nullptr;
    }
    FFPacket* ffpkt = pktQueue.front();
    pktQueue.pop();
    cond.notify_one();
//    std::cerr<<"vpacket size:"<<pktQueue.size()<<std::endl;
    return ffpkt;
}

FFPacket *FFVPacketQueue::peekQueue()
{
    std::lock_guard<std::mutex>lock(mutex);
    return pktQueue.empty()? nullptr : pktQueue.front();
}

void FFVPacketQueue::enqueue(AVPacket *pkt)
{
    std::unique_lock<std::mutex> lock(mutex);
//    if(pktQueue.size() == MAX_PACKET_SIZE){
//        av_packet_unref(pkt);
//        std::cerr << "video packet droped !" << std::endl;
//        return;
//    }
    cond.wait(lock,[this]() { return pktQueue.size() < MAX_PACKET_SIZE  || m_stop.load(); });
    if(m_stop.load()){
        std::cout <<"stop load!" <<std::endl;
        av_packet_unref(pkt);
        m_stop.store(false);
        return;
    }
    FFPacket* ffpkt = static_cast<FFPacket*>(av_mallocz(sizeof(FFPacket)));

    av_init_packet(&ffpkt->packet);
    av_packet_move_ref(&ffpkt->packet,pkt);
    ffpkt->serial = serial;
    ffpkt->type = NORMAL;

    pktQueue.push(ffpkt);
    cond.notify_one();
//    std::cerr << "enqueue vpacket size:" << pktQueue.size() << " id:" <<std::this_thread::get_id()<< std::endl;
}

void FFVPacketQueue::enqueueFlush()
{

    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock,[this]() { return pktQueue.size() < MAX_PACKET_SIZE || m_stop.load(); });

    if(m_stop){
        return;
    }

    FFPacket* ffpkt = static_cast<FFPacket*>(av_mallocz(sizeof(FFPacket)));
    av_init_packet(&ffpkt->packet);
    ffpkt->type = FLUSH;
    ffpkt->serial = this->serial++;
    pktQueue.push(ffpkt);
    cond.notify_one();
}

void FFVPacketQueue::enqueueNull()
{
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock,[this]() { return pktQueue.size() < MAX_PACKET_SIZE || m_stop.load(); });

    if(m_stop){
        return;
    }

    FFPacket* ffpkt = static_cast<FFPacket*>(av_mallocz(sizeof(FFPacket)));
    av_init_packet(&ffpkt->packet);
    ffpkt->type = NULLP;
    ffpkt->serial = this->serial;
    ffpkt->packet.data = nullptr;
    pktQueue.push(ffpkt);

    cond.notify_one();
}

void FFVPacketQueue::flushQueue()
{

    while(1){
        FFPacket* pkt = peekQueue();
        if(pkt == nullptr || pkt->serial == this->serial){
            break;
        }
        else{
            std::lock_guard<std::mutex>lock(mutex);
            pktQueue.pop();

            av_packet_unref(&pkt->packet);
            av_freep(&pkt);
        }
    }
    cond.notify_one();
}
size_t FFVPacketQueue::getSerial()
{
    return serial.load();
}

void FFVPacketQueue::clearQueue()
{
    std::lock_guard<std::mutex>lock(mutex);
    while(!pktQueue.empty()){
        FFPacket* ffpkt = pktQueue.front();
        pktQueue.pop();
        if(ffpkt!=nullptr){
            av_packet_unref(&ffpkt->packet);
            av_freep(&ffpkt);
        }
    }

}

void FFVPacketQueue::close()
{
    wakeAllThread();
    clearQueue();
}

void FFVPacketQueue::start()
{
    m_stop = false;
}

void FFVPacketQueue::setMaxSize(size_t maxSize_)
{
    maxSize = maxSize_;
}

void FFVPacketQueue::wakeAllThread()
{
    m_stop = true;
    cond.notify_all();
}
