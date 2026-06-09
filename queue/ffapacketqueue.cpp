#include "ffapacketqueue.h"
#include"ffpacket.h"

#define MAX_PACKET_SIZE 3

FFAPacketQueue::FFAPacketQueue()
    : serial(0),m_stop(false)
{
}

FFAPacketQueue::~FFAPacketQueue()
{
    close();
}

FFPacket* FFAPacketQueue::dequeue()
{
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock,[this]() { return !pktQueue.empty() || m_stop.load(); });
    if(m_stop.load()){
        return nullptr;
    }
    FFPacket* ffpkt = pktQueue.front();
    pktQueue.pop();
    cond.notify_one();
//    std::cout << "dequeue apacket! "<< std::endl;
    return ffpkt;
}

FFPacket *FFAPacketQueue::peekQueue()
{
    std::lock_guard<std::mutex>lock(mutex);
    return pktQueue.empty()? nullptr : pktQueue.front();
}

void FFAPacketQueue::enqueue(AVPacket *pkt)
{
    std::unique_lock<std::mutex> lock(mutex);
//    if(pktQueue.size() == MAX_PACKET_SIZE){
//        av_packet_unref(pkt);
////        std::cerr <<"drop audio packet!" << std::endl;
//        return;
//    }
    cond.wait(lock,[this]() { return pktQueue.size() < MAX_PACKET_SIZE  || m_stop.load(); });
    if(m_stop.load()){
        std::cerr<<"m_stop load!"<<std::endl;
        av_packet_unref(pkt);
        return;
    }
    FFPacket* ffpkt = static_cast<FFPacket*>(av_mallocz(sizeof(FFPacket)));
    av_init_packet(&ffpkt->packet);
    av_packet_move_ref(&ffpkt->packet,pkt);
    ffpkt->serial = serial;
    ffpkt->type = NORMAL;

    pktQueue.push(ffpkt);
//    std::cout << "enqueue apacket: "<< pktQueue.size()<< std::endl;
    cond.notify_one();
}

void FFAPacketQueue::enqueueFlush()
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

void FFAPacketQueue::enqueueNull()
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

void FFAPacketQueue::flushQueue()
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

size_t FFAPacketQueue::getSerial()
{
    return serial.load();
}

void FFAPacketQueue::clearQueue()
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

void FFAPacketQueue::wakeAllThread()
{
    m_stop = true;
    cond.notify_all();
}

void FFAPacketQueue::close()
{
    wakeAllThread();
    clearQueue();
}

void FFAPacketQueue::start()
{
    m_stop = false;
}

