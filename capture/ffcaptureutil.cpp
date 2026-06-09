#include "ffcaptureutil.h"
#include "ui/ffcapwindow.h"
#include "demuxer/ffdemuxer.h"
#include "queue/ffapacketqueue.h"
#include "queue/ffvpacketqueue.h"
#include "queue/ffvframequeue.h"
#include "queue/ffaframequeue.h"
//#include "render/ffvrender.h"
#include "thread/ffvdecoderthread.h"
#include "thread/ffdemuxerthread.h"
#include "thread/ffadecoderthread.h"
#include "decoder/ffadecoder.h"
#include "decoder/ffvdecoder.h"
#include "muxer/ffmuxer.h"
#include "thread/ffamuxerthread.h"
#include "thread/ffvmuxerthread.h"
#include "thread/ffmuxerthread.h"
#include "encoder/ffaencoder.h"
#include "encoder/ffvencoder.h"
#include "thread/ffaencoderthread.h"
#include "thread/ffvencoderthread.h"
#include "filter/ffvfilter.h"
#include "filter/ffafilter.h"
#include "thread/ffvfilterthread.h"
#include "thread/ffafilterthread.h"
#include "thread/ffthreadpool.h"
#include "event/ffeventloop.h"
#include "queue/ffeventqueue.h"
#include "capture/ffcapturecontext.h"

#include <QDebug>

FFCaptureUtil::FFCaptureUtil(QObject *parent) : QObject(parent)
{
    // 初始化FFmpeg库
    avdevice_register_all();
    avformat_network_init();
}

FFCaptureUtil::~FFCaptureUtil()
{
#if 0
    stopCapture();

    // 释放资源
    delete m_capWindow;
    delete m_captureCtx;
    delete m_threadPool;
    delete m_eventLoop;

    // 释放解复用器
    delete m_vDemuxer1;
    delete m_vDemuxer2;
    delete m_vDemuxer3;
    delete m_aDemuxer1;
    delete m_aDemuxer2;

    // 释放队列
    delete m_aPktQueue1;
    delete m_aPktQueue2;
    delete m_vPktQueue1;
    delete m_vPktQueue2;
    delete m_vPktQueue3;
    delete m_vFrmQueue1;
    delete m_vFrmQueue2;
    delete m_vFrmQueue3;
    delete m_vRenderFrmQueue;
    delete m_aFrmQueue1;
    delete m_aFrmQueue2;
    delete m_vFilterEncoderFrmQueue;
    delete m_aFilterEncoderFrmQueue;
    delete m_aEncoderPktQueue;
    delete m_vEncoderPktQueue;

    // 释放解码器
    delete m_vDecoder1;
    delete m_vDecoder2;
    delete m_vDecoder3;
    delete m_aDecoder1;
    delete m_aDecoder2;

    // 释放编码器
    delete m_aEncoder;
    delete m_vEncoder;

    // 释放过滤器
    delete m_vFilter;
    delete m_aFilter;

//    // 释放渲染器
//    delete m_vRender;

    // 释放复用器
    delete m_muxer;

    // 释放线程
    delete m_vDemuxerThread1;
    delete m_vDemuxerThread2;
    delete m_vDemuxerThread3;
    delete m_aDemuxerThread1;
    delete m_aDemuxerThread2;
    delete m_vDecoderThread1;
    delete m_vDecoderThread2;
    delete m_vDecoderThread3;
    delete m_aDecoderThread1;
    delete m_aDecoderThread2;
    delete m_aEncoderThread;
    delete m_vEncoderThread;
    delete m_muxerThread;
    delete m_vFilterThread;
    delete m_aFilterThread;
#endif
}

void FFCaptureUtil::initialize()
{
    if (m_isCapturing)
        return;
    av_log_set_level(AV_LOG_QUIET);

    registerMetaTypes();
    initCoreComponents();
    initCaptureContext();
}

void FFCaptureUtil::startCapture()
{
    if (m_isCapturing)
        return;

    if (!m_capWindow || !m_captureCtx) {
        qCritical() << "FFCaptureUtil not initialized!";
        return;
    }

    m_capWindow->init(m_captureCtx);
    m_capWindow->show();

    // 启动过滤器线程
    m_vFilterThread->start();
    m_aFilterThread->start();

    // 启动事件循环
    m_eventLoop->start();

    m_isCapturing = true;
}

void FFCaptureUtil::stopCapture()
{
    if (!m_isCapturing)
        return;

    if (m_vFilterThread){
        m_vFilterThread->stop();
        m_vFilterThread->wakeAllThread();
        m_vFilterThread->wait();
    }
    if (m_aFilterThread){
        m_aFilterThread->stop();
        m_aFilterThread->wakeAllThread();
        m_aFilterThread->wait();
    }

    // 停止所有线程
    if (m_eventLoop){
        m_eventLoop->stop();
        m_eventLoop->wakeAllThread();
        m_eventLoop->wait();
    }

    // 隐藏窗口
    if (m_capWindow){
        m_capWindow->hide();
    }


    m_isCapturing = false;
}

void FFCaptureUtil::initCoreComponents()
{
    // 初始化窗口
    m_capWindow = new FFCapWindow();

    // 初始化队列
    m_aPktQueue1 = new FFAPacketQueue();
    m_aPktQueue2 = new FFAPacketQueue();
    m_vPktQueue1 = new FFVPacketQueue();
    m_vPktQueue2 = new FFVPacketQueue();
    m_vPktQueue3 = new FFVPacketQueue();
    m_vFrmQueue1 = new FFVFrameQueue();
    m_vFrmQueue2 = new FFVFrameQueue();
    m_vFrmQueue3 = new FFVFrameQueue();
    m_vRenderFrmQueue = new FFVFrameQueue();
    m_aFrmQueue1 = new FFAFrameQueue();
    m_aFrmQueue2 = new FFAFrameQueue();
    m_vFilterEncoderFrmQueue = new FFVFrameQueue();
    m_aFilterEncoderFrmQueue = new FFAFrameQueue();
    m_aEncoderPktQueue = new FFAPacketQueue();
    m_vEncoderPktQueue = new FFVPacketQueue();

    // 初始化解复用器
    m_vDemuxer1 = new FFDemuxer();
    m_vDemuxer2 = new FFDemuxer();
    m_vDemuxer3 = new FFDemuxer();
    m_aDemuxer1 = new FFDemuxer();
    m_aDemuxer2 = new FFDemuxer();

    // 初始化解码器
    m_vDecoder1 = new FFVDecoder();
    m_vDecoder2 = new FFVDecoder();
    m_vDecoder3 = new FFVDecoder();
    m_aDecoder1 = new FFADecoder();
    m_aDecoder2 = new FFADecoder();

    // 初始化编码器
    m_aEncoder = new FFAEncoder();
    m_vEncoder = new FFVEncoder();
    m_aEncoder->init(m_aEncoderPktQueue);
    m_vEncoder->init(m_vEncoderPktQueue);

    // 初始化过滤器
    m_vFilter = new FFVFilter();
    m_aFilter = new FFAFilter();
    m_vFilter->init(m_vFilterEncoderFrmQueue, m_vDecoder1, m_vDecoder2);
    m_aFilter->init(m_aFilterEncoderFrmQueue, m_aDecoder1, m_aDecoder2);

//    // 初始化渲染器
//    m_vRender = new FFVRender();
//    m_vRender->init(m_vRenderFrmQueue, m_capWindow);

    // 初始化复用器
    m_muxer = new FFMuxer();

    // 初始化线程
    m_vDemuxerThread1 = new FFDemuxerThread();
    m_vDemuxerThread2 = new FFDemuxerThread();
    m_vDemuxerThread3 = new FFDemuxerThread();
    m_aDemuxerThread1 = new FFDemuxerThread();
    m_aDemuxerThread2 = new FFDemuxerThread();
    m_vDecoderThread1 = new FFVDecoderThread();
    m_vDecoderThread2 = new FFVDecoderThread();
    m_vDecoderThread3 = new FFVDecoderThread();
    m_aDecoderThread1 = new FFADecoderThread();
    m_aDecoderThread2 = new FFADecoderThread();
    m_aEncoderThread = new FFAEncoderThread();
    m_vEncoderThread = new FFVEncoderThread();
    m_muxerThread = new FFMuxerThread();
    m_vFilterThread = new FFVFilterThread();
    m_aFilterThread = new FFAFilterThread();

    // 初始化过滤器线程
    m_vFilterThread->init(m_vFrmQueue1, m_vFrmQueue2, m_vFrmQueue3, m_vRenderFrmQueue, m_vFilter, m_capWindow);
    m_aFilterThread->init(m_aFrmQueue1, m_aFrmQueue2, m_aFilter);

    // 初始化线程池和事件循环
    m_threadPool = new FFThreadPool();
    m_threadPool->init(4);
    m_eventLoop = new FFEventLoop();
    m_eventLoop->init(&FFEventQueue::getInstance(), m_threadPool);

    // 设置窗口
    m_capWindow->setVFilter(m_vFilter);
//    m_capWindow->setVRender(m_vRender);
}

void FFCaptureUtil::initCaptureContext()
{
    m_captureCtx = new FFCaptureContext();

    m_captureCtx->capWindow = m_capWindow;
    m_captureCtx->muxer = m_muxer;
    m_captureCtx->muxerThread = m_muxerThread;
    m_captureCtx->aFilter = m_aFilter;
    m_captureCtx->aFilterThread = m_aFilterThread;
    m_captureCtx->vFilter = m_vFilter;
    m_captureCtx->vFilterThread = m_vFilterThread;
//    m_captureCtx->vRender = m_vRender;

    // 设置音频解码器
    using namespace FFCaptureContextType;
    m_captureCtx->aDecoder[demuxerIndex[AUDIO]] = m_aDecoder1;
    m_captureCtx->aDecoder[demuxerIndex[MICROPHONE]] = m_aDecoder2;

    // 设置音频解复用器
    m_captureCtx->aDemuxer[demuxerIndex[AUDIO]] = m_aDemuxer1;
    m_captureCtx->aDemuxer[demuxerIndex[MICROPHONE]] = m_aDemuxer2;

    // 设置音频解复用线程
    m_captureCtx->aDemuxerThread[demuxerIndex[AUDIO]] = m_aDemuxerThread1;
    m_captureCtx->aDemuxerThread[demuxerIndex[MICROPHONE]] = m_aDemuxerThread2;

    // 设置音频解码线程
    m_captureCtx->aDecoderThread[demuxerIndex[AUDIO]] = m_aDecoderThread1;
    m_captureCtx->aDecoderThread[demuxerIndex[MICROPHONE]] = m_aDecoderThread2;

    // 设置音频编码器
    m_captureCtx->aEncoder = m_aEncoder;
    m_captureCtx->aEncoderThread = m_aEncoderThread;

    // 设置视频解码器
    m_captureCtx->vDecoder[demuxerIndex[SCREEN]] = m_vDecoder1;
    m_captureCtx->vDecoder[demuxerIndex[CAMERA]] = m_vDecoder2;
    m_captureCtx->vDecoder[demuxerIndex[VIDEO]] = m_vDecoder3;

    // 设置视频解复用器
    m_captureCtx->vDemuxer[demuxerIndex[SCREEN]] = m_vDemuxer1;
    m_captureCtx->vDemuxer[demuxerIndex[CAMERA]] = m_vDemuxer2;
    m_captureCtx->vDemuxer[demuxerIndex[VIDEO]] = m_vDemuxer3;

    // 设置视频解复用线程
    m_captureCtx->vDemuxerThread[demuxerIndex[SCREEN]] = m_vDemuxerThread1;
    m_captureCtx->vDemuxerThread[demuxerIndex[CAMERA]] = m_vDemuxerThread2;
    m_captureCtx->vDemuxerThread[demuxerIndex[VIDEO]] = m_vDemuxerThread3;

    // 设置视频解码线程
    m_captureCtx->vDecoderThread[demuxerIndex[SCREEN]] = m_vDecoderThread1;
    m_captureCtx->vDecoderThread[demuxerIndex[CAMERA]] = m_vDecoderThread2;
    m_captureCtx->vDecoderThread[demuxerIndex[VIDEO]] = m_vDecoderThread3;

    // 设置视频编码器
    m_captureCtx->vEncoder = m_vEncoder;
    m_captureCtx->vEncoderThread = m_vEncoderThread;

    // 设置音频队列
    m_captureCtx->aDecoderPktQueue[demuxerIndex[AUDIO]] = m_aPktQueue1;
    m_captureCtx->aDecoderPktQueue[demuxerIndex[MICROPHONE]] = m_aPktQueue2;
    m_captureCtx->aDecoderFrmQueue[demuxerIndex[AUDIO]] = m_aFrmQueue1;
    m_captureCtx->aDecoderFrmQueue[demuxerIndex[MICROPHONE]] = m_aFrmQueue2;
    m_captureCtx->aEncoderPktQueue = m_aEncoderPktQueue;

    // 设置视频队列
    m_captureCtx->vDecoderPktQueue[demuxerIndex[SCREEN]] = m_vPktQueue1;
    m_captureCtx->vDecoderPktQueue[demuxerIndex[CAMERA]] = m_vPktQueue2;
    m_captureCtx->vDecoderPktQueue[demuxerIndex[VIDEO]] = m_vPktQueue3;
    m_captureCtx->vDecoderFrmQueue[demuxerIndex[SCREEN]] = m_vFrmQueue1;
    m_captureCtx->vDecoderFrmQueue[demuxerIndex[CAMERA]] = m_vFrmQueue2;
    m_captureCtx->vDecoderFrmQueue[demuxerIndex[VIDEO]] = m_vFrmQueue3;
    m_captureCtx->vRenderFrmQueue = m_vRenderFrmQueue;
    m_captureCtx->vFilterEncoderFrmQueue = m_vFilterEncoderFrmQueue;
    m_captureCtx->vEncoderPktQueue = m_vEncoderPktQueue;
    m_captureCtx->aFilterEncoderFrmQueue = m_aFilterEncoderFrmQueue;
}


void FFCaptureUtil::registerMetaTypes()
{
    qRegisterMetaType<uint8_t*>("uint8_t*");
    qRegisterMetaType<AVFrame*>("AVFrame*");
    qRegisterMetaType<int64_t>("int64_t");
}
