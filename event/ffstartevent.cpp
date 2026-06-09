#include "ffstartevent.h"

FFStartEvent::FFStartEvent(FFCaptureContext *captureCtx,
                           std::string const& url_,std::string const& format_)
    :FFEvent (captureCtx)
{
    url = url_;
    format = format_;
}

void FFStartEvent::work()
{
    initEncoder();
    startQueue();
    startEncoder();
}

void FFStartEvent::initEncoder()
{
    //初始化复用线程
    muxer->init(url,format);
    muxerThread->init(aEncoderPktQueue,vEncoderPktQueue,muxer,aEncoder,vEncoder,captureContext);

    //    std::cout << "muxer init" <<std::endl;
    //初始化编码线程
    aEncoder->init(aEncoderPktQueue);
    aEncoderThread->init(aFilter,aEncoder,muxer,aFilterEncoderFrmQueue);

    //     std::cout << "aEncoder init" <<std::endl;

    vEncoder->init(vEncoderPktQueue);
    vEncoderThread->init(vFilter,vEncoder,muxer,vFilterEncoderFrmQueue);

    //     std::cout << "vEncoder init" <<std::endl;
}

void FFStartEvent::startQueue()
{
    //开启队列
    aEncoderPktQueue->start();
    vEncoderPktQueue->start();

    aFilterEncoderFrmQueue->start();
    vFilterEncoderFrmQueue->start();
}


void FFStartEvent::startEncoder()
{

    std::cout << "startEncoder" << std::endl;

    //开启编码标志
    aFilterThread->startEncoder();
    vFilterThread->startEncoder();

    //开启编码线程
    //    aEncoderThread->wakeAllThread();
    aEncoderThread->start();

    std::cout << "aEncoderThread start" << std::endl;

    //    vEncoderThread->wakeAllThread();
    vEncoderThread->start();

    std::cout << "vEncoderThread start" << std::endl;

    //开启复用线程
    //    muxerThread->wakeAllThread();
    muxerThread->start();

    std::cout << "muxerThread start" << std::endl;

}


