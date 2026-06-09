#include "ffvframequeue.h"

#define MAX_FRAME_SIZE 2

FFVFrameQueue::FFVFrameQueue()
    : m_stop(false) {
}

FFVFrameQueue::~FFVFrameQueue() {
    close();
}

void FFVFrameQueue::enqueue(AVFrame* srcFrame) {
    std::unique_lock<std::mutex> lock(mutex);
//    if(frmQueue.size() == MAX_FRAME_SIZE){
//        av_frame_unref(srcFrame);
//        return;
//    }
    cond.wait(lock, [this]() { return frmQueue.size() < MAX_FRAME_SIZE || m_stop.load(); });

    if (m_stop.load()) {
        av_frame_unref(srcFrame);
        return;
    }

    AVFrame* destFrame = av_frame_alloc();
    if (!destFrame) {
        av_frame_unref(srcFrame);
        return;
    }
    av_frame_move_ref(destFrame, srcFrame);
    av_frame_unref(srcFrame);
    frmQueue.push(destFrame);
    cond.notify_one();
}

void FFVFrameQueue::enqueueNull() {
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [this]() { return frmQueue.size() < MAX_FRAME_SIZE || m_stop.load(); });

    if (m_stop.load()) {
        return;
    }

    AVFrame* frame = av_frame_alloc();
    if (frame) {
        frame->data[0] = nullptr;
        frame->data[1] = nullptr;
        frame->data[2] = nullptr;
        frmQueue.push(frame);
        cond.notify_one();
    }
}

AVFrame* FFVFrameQueue::dequeue() {
    std::unique_lock<std::mutex> lock(mutex);
    if(m_stop){
        return nullptr;
    }
    cond.wait(lock, [this]() { return !frmQueue.empty() || m_stop.load(); });

    if (m_stop.load()) {
        return nullptr;
    }

    AVFrame* frame = frmQueue.front();
    frmQueue.pop();
    cond.notify_one();
//    std::cout<<"frameQueue Size:"<<frmQueue.size()<<std::endl;
    return frame;
}

void FFVFrameQueue::wakeAllThread() {
    m_stop.store(true, std::memory_order_release);
    cond.notify_all();
}

void FFVFrameQueue::clearQueue() {
    std::lock_guard<std::mutex> lock(mutex);
    while (!frmQueue.empty()) {
        AVFrame* frame = frmQueue.front();
        frmQueue.pop();
        if (frame) {
            av_frame_unref(frame);
            av_frame_free(&frame);
        }
    }
}

void FFVFrameQueue::flushQueue(){
    std::lock_guard<std::mutex>lock(mutex);
    while(1){
        AVFrame *frame = peekQueue();
        if(frame == nullptr){
            break;
        }
        frmQueue.pop();
        av_frame_unref(frame);
        av_frame_free(&frame);

    }
//     std::cerr<<"flush vframe Queue!"<<std::endl;
    cond.notify_one();
}

void FFVFrameQueue::close()
{
    wakeAllThread();
    clearQueue();
}

void FFVFrameQueue::start()
{
    m_stop = false;
}

bool FFVFrameQueue::peekEmpty()
{
    std::lock_guard<std::mutex> lock(mutex);
    return frmQueue.empty();

}

AVFrame* FFVFrameQueue::peekQueue()
{
    return frmQueue.empty() ? nullptr: frmQueue.front();
}
