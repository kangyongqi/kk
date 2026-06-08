#ifndef FFCAPTURECONTEXT_H
#define FFCAPTURECONTEXT_H

#include"ui/ffcapwindow.h"

//#include"render/ffvrender.h"

#include"thread/ffadecoderthread.h"
#include"thread/ffvdecoderthread.h"
#include"thread/ffaencoderthread.h"
#include"thread/ffvencoderthread.h"
#include"thread/ffafilterthread.h"
#include"thread/ffvfilterthread.h"
#include"thread/ffmuxerthread.h"
#include"thread/ffdemuxerthread.h"

#include"filter/ffafilter.h"
#include"filter/ffvfilter.h"

#include"demuxer/ffdemuxer.h"

#include"encoder/ffaencoder.h"
#include"encoder/ffvencoder.h"

#include"muxer/ffmuxer.h"

namespace FFCaptureContextType{

constexpr size_t A_DECODER_SIZE = 2;
constexpr size_t A_DEMUXER_SIZE = 2;
constexpr size_t V_DECODER_SIZE = 3;
constexpr size_t V_DEMUXER_SIZE = 3;

enum demuxerType{
    SCREEN,CAMERA,VIDEO,AUDIO,MICROPHONE,NOTYPE
};

constexpr int demuxerIndex[A_DEMUXER_SIZE+V_DEMUXER_SIZE + 1]{
    0,1,2,
    0,1,-1
};

};

class FFCaptureContext
{
public:
    FFCaptureContext();
    ~FFCaptureContext();

public:
    //音频解码线程：【声卡】、【麦克风】
    FFADecoderThread* aDecoderThread[FFCaptureContextType::A_DECODER_SIZE];

    //视频解码线程：【屏幕】、【摄像头】、【视频】
    FFVDecoderThread* vDecoderThread[FFCaptureContextType::V_DECODER_SIZE];

    //音频解复用线程：【音频】、【麦克风】
    FFDemuxerThread* aDemuxerThread[FFCaptureContextType::A_DEMUXER_SIZE];
    //视频解复用线程 【屏幕】、【摄像头】、【视频】
    FFDemuxerThread* vDemuxerThread[FFCaptureContextType::V_DEMUXER_SIZE];

    //音频解码packet包队列
    FFAPacketQueue* aDecoderPktQueue[FFCaptureContextType::A_DECODER_SIZE];

    //视频解码packet包队列
    FFVPacketQueue* vDecoderPktQueue[FFCaptureContextType::V_DECODER_SIZE];

    //视频编码packet包队列
    FFVPacketQueue* vEncoderPktQueue;

    //音频编码packet包队列
    FFAPacketQueue* aEncoderPktQueue;

    //音频解码帧队列
    FFAFrameQueue* aDecoderFrmQueue[FFCaptureContextType::A_DECODER_SIZE];

    //视频解码帧队列
    FFVFrameQueue* vDecoderFrmQueue[FFCaptureContextType::V_DECODER_SIZE];

    //视频filter编码Frame队列
    FFVFrameQueue* vFilterEncoderFrmQueue;

    //音频filter编码Frame队列
    FFAFrameQueue* aFilterEncoderFrmQueue;

    //视频渲染帧队列：【视频文件】
    FFVFrameQueue* vRenderFrmQueue;

    //音频解复用器
    FFDemuxer* aDemuxer[FFCaptureContextType::A_DEMUXER_SIZE];

    //视频解复用器
    FFDemuxer* vDemuxer[FFCaptureContextType::V_DEMUXER_SIZE];

    //音频解码器
    FFADecoder* aDecoder[FFCaptureContextType::A_DECODER_SIZE];

    //视频解码器
    FFVDecoder* vDecoder[FFCaptureContextType::V_DECODER_SIZE];

//    //视频渲染器
//    FFVRender* vRender;

    //复用器
    FFMuxer* muxer;

    //复用线程
    FFMuxerThread* muxerThread;

    //音频编码器
    FFAEncoder* aEncoder;

    //音频编码线程
    FFAEncoderThread * aEncoderThread;

    //视频编码器
    FFVEncoder* vEncoder;

    //视频编码线程
    FFVEncoderThread* vEncoderThread;

    //视频过滤器
    FFVFilter* vFilter;

    //视频过滤线程
    FFVFilterThread* vFilterThread;

    //音频过滤器
    FFAFilter* aFilter;

    //音频滤镜线程
    FFAFilterThread* aFilterThread;




    //UI窗口
    FFCapWindow* capWindow;
};


#endif
