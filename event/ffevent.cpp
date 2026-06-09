#include"ffevent.h"


FFEvent::FFEvent(FFCaptureContext *captureCtx):
    captureContext(captureCtx)
{
    //过滤器
    aFilter = captureCtx->aFilter;
    vFilter = captureCtx->vFilter;

    //过滤线程
    aFilterThread = captureCtx->aFilterThread;
    vFilterThread = captureCtx->vFilterThread;

    //音频包队列、帧队列、解码器、解码线程、解复用器、解复用线程
    for(size_t i = 0 ; i< A_DEMUXER_SIZE; ++i){
        aPktQueue[i] = captureCtx->aDecoderPktQueue[i];
        aFrmQueue[i] = captureCtx->aDecoderFrmQueue[i];

        aDecoder[i] = captureCtx->aDecoder[i];
        aDecoderThread[i] = captureCtx->aDecoderThread[i];

        aDemuxer[i] = captureCtx->aDemuxer[i];
        aDemuxerThread[i] = captureCtx->aDemuxerThread[i];
    }

    //视频包队列、帧队列、解码器、解码线程、解复用器、解复用线程
    for(size_t i = 0 ; i< V_DEMUXER_SIZE; ++i){
        vPktQueue[i] = captureCtx->vDecoderPktQueue[i];
        vFrmQueue[i] = captureCtx->vDecoderFrmQueue[i];

        vDecoder[i] = captureCtx->vDecoder[i];
        vDecoderThread[i] = captureCtx->vDecoderThread[i];

        vDemuxer[i] = captureCtx->vDemuxer[i];
        vDemuxerThread[i] = captureCtx->vDemuxerThread[i];
    }


    //音视频编码包队列
    aEncoderPktQueue = captureCtx->aEncoderPktQueue;
    vEncoderPktQueue = captureCtx->vEncoderPktQueue;

    //音视频编码帧队列
    aFilterEncoderFrmQueue = captureCtx->aFilterEncoderFrmQueue;
    vFilterEncoderFrmQueue = captureCtx->vFilterEncoderFrmQueue;

    //音视频编码器
    aEncoder = captureCtx->aEncoder;
    vEncoder = captureCtx->vEncoder;

    //音视频编码队列
    aEncoderThread = captureCtx->aEncoderThread;
    vEncoderThread = captureCtx->vEncoderThread;

    //复用器、复用线程
    muxer = captureCtx->muxer;
    muxerThread = captureContext->muxerThread;

    //视频渲染帧队列
    vRenderFrmQueue = captureCtx->vRenderFrmQueue;

    //UI窗口
    capWindow = captureContext->capWindow;

//    //视频渲染器
//    vRender = captureCtx->vRender;
}

FFEvent::~FFEvent(){}



