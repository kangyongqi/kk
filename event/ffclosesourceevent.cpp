#include "ffclosesourceevent.h"

using namespace FFCaptureContextType;

FFCloseSourceEvent::FFCloseSourceEvent(FFCaptureContext *captureCtx, FFCaptureContextType::demuxerType sourceType_)
    :FFEvent (captureCtx)
{
    index = demuxerIndex[sourceType_];
    sourceType = sourceType_;
}

void FFCloseSourceEvent::work()
{

    close();
    std::cout << "Source Type:" << sourceType<<std::endl;
}

void FFCloseSourceEvent::close()
{
    //关闭解复用线程
    //    std::cout << "demuxer type" << demuxer->getType()<< std::endl;

    //关闭解码线程
    if(sourceType == AUDIO || sourceType == MICROPHONE){
        aDemuxerThread[index]->stop();
        // 设置线程内部的停止标志
        aDemuxerThread[index]->wakeAllThread();
        // 唤醒可能在等待的线程（比如在队列的 dequeue() 中阻塞）
        aDemuxerThread[index]->wait();
        // 避免主线程在子线程还没退出时就继续执行
        aDemuxerThread[index]->close();
        // 确保线程的所有资源都被正确释放
        aDecoderThread[index]->stop();
        aDecoderThread[index]->wakeAllThread();
        aDecoderThread[index]->wait();
        // 避免主线程在子线程还没退出时就继续执行
        aDecoderThread[index]->close();
    }
    else{
       vDemuxerThread[index]->stop();
       vDemuxerThread[index]->wakeAllThread();

       vDemuxerThread[index]->wait();
       vDemuxerThread[index]->close();

       vDecoderThread[index]->stop();
       vDecoderThread[index]->wakeAllThread();
       vDecoderThread[index]->wait();
       vDecoderThread[index]->close();
    }


    //清空对应的队列
    if(sourceType == AUDIO || sourceType == MICROPHONE){
        aPktQueue[index]->close();
        aFrmQueue[index]->close();
    }
    else{
        vPktQueue[index]->close();
        vFrmQueue[index]->close();
    }

    //设置关闭标志
    if(sourceType == AUDIO || sourceType == MICROPHONE){
        aFilterThread->closeAudioSource(sourceType);
    }
    else{
        vFilterThread->closeVideoSource(sourceType);
    }

}


