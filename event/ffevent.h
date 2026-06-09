 #ifndef FFEVENT_H
#define FFEVENT_H

#include<QMetaObject>
#include"thread/ffdemuxerthread.h"
#include"thread/ffadecoderthread.h"
#include"thread/ffvdecoderthread.h"
#include"capture/ffcapturecontext.h"
#include"queue/ffapacketqueue.h"
#include"queue/ffvpacketqueue.h"
#include"queue/ffaframequeue.h"
#include"queue/ffvframequeue.h"

#include"decoder/ffadecoder.h"
#include"decoder/ffvdecoder.h"
#include"demuxer/ffdemuxer.h"

using namespace FFCaptureContextType;
class FFEvent
{
public:
    FFEvent(FFCaptureContext* captureContext_);
    virtual ~FFEvent();

    virtual void work() = 0;

protected:
    //全局上下文
    FFCaptureContext* captureContext = nullptr;

    //音视频包队列
    FFAPacketQueue* aPktQueue[A_DEMUXER_SIZE];
    FFVPacketQueue* vPktQueue[V_DEMUXER_SIZE];

    //音视频帧队列
    FFAFrameQueue* aFrmQueue[A_DECODER_SIZE];
    FFVFrameQueue* vFrmQueue[V_DECODER_SIZE];

    //视频渲染帧队列
    FFVFrameQueue* vRenderFrmQueue;

    //编码线程
    FFAFrameQueue* aFilterEncoderFrmQueue;
    FFVFrameQueue* vFilterEncoderFrmQueue;

    //解码线程
    FFAPacketQueue* aEncoderPktQueue;
    FFVPacketQueue* vEncoderPktQueue;

    //过滤器
    FFAFilter* aFilter;
    FFVFilter* vFilter;

    //过滤线程
    FFAFilterThread* aFilterThread;
    FFVFilterThread* vFilterThread;

    //解码器
    FFADecoder* aDecoder[A_DECODER_SIZE];
    FFVDecoder* vDecoder[V_DECODER_SIZE];

    //UI窗口
    FFCapWindow* capWindow;

    //编码器
    FFAEncoder* aEncoder;
    FFVEncoder* vEncoder;

    //编码线程
    FFAEncoderThread* aEncoderThread;
    FFVEncoderThread* vEncoderThread;

    //复用器
    FFMuxer* muxer;

    //复用线程
    FFMuxerThread* muxerThread;

    //解码线程
    FFADecoderThread* aDecoderThread[A_DECODER_SIZE];
    FFVDecoderThread* vDecoderThread[V_DECODER_SIZE];

    //解复用器
    FFDemuxer* aDemuxer[A_DEMUXER_SIZE];
    FFDemuxer* vDemuxer[V_DEMUXER_SIZE];

    //解复用线程
    FFDemuxerThread* aDemuxerThread[A_DEMUXER_SIZE];
    FFDemuxerThread* vDemuxerThread[V_DEMUXER_SIZE];

//    //视频渲染器
//    FFVRender* vRender =nullptr;
};

#endif // FFEVENT_H

