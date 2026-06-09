#include "ffstopevent.h"

FFStopEvent::FFStopEvent(FFCaptureContext* captureCtx)
    :FFEvent (captureCtx)
{

}

void FFStopEvent::work()
{
    close();
    clearQueue();
}

void FFStopEvent::close()
{
    //è®¾ç½®ç¼–ç æ ‡å¿—
    vFilterThread->stopEncoder();
    aFilterThread->stopEncoder();

    //å…³é—­ç¼–ç çº¿ç¨‹
    vEncoderThread->stop();
    vEncoderThread->wakeAllThread();
    vEncoderThread->wait();
    vEncoderThread->close();

    aEncoderThread->stop();
    aEncoderThread->wakeAllThread();
    aEncoderThread->wait();
    aEncoderThread->close();

    //å…³é—­å¤ç”¨çº¿ç¨‹
    muxerThread->stop();
    muxerThread->wakeAllThread();
    muxerThread->wait();
    muxerThread->close();

}

void FFStopEvent::clearQueue()
{   // ±àÂëÆ÷Êä³öµÄ±àÂë°ü¶ÓÁÐ
    aEncoderPktQueue->close();
    vEncoderPktQueue->close();
    // ±àÂëÆ÷µÄÊäÈë
    aFilterEncoderFrmQueue->close();
    vFilterEncoderFrmQueue->close();
}
