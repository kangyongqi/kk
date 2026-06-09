#ifndef FFCAPTUREUTIL_H
#define FFCAPTUREUTIL_H

#include <QObject>
#include <QIcon>
#include <QApplication>

class FFCapWindow;
class FFDemuxer;
class FFADecoder;
class FFVDecoder;
//class FFVRender;
class FFMuxer;
class FFAEncoder;
class FFVEncoder;
class FFVFilter;
class FFAFilter;
class FFEventLoop;
class FFThreadPool;
class FFCaptureContext;

namespace FFCaptureURLS{
    static std::string CAMERA1_URL = "video=ov9734_techfront_camera";
    static std::string CAMERA2_URL = "video=icspring camera";//外接摄像头
    static std::string SCREEN_URL = "video=screen-capture-recorder";
    static std::string AUDIO_URL = "audio=virtual-audio-capturer";
    static std::string MICROPHONE_URL = "audio=麦克风 (Realtek(R) Audio)";
};

class FFCaptureUtil : public QObject
{
    Q_OBJECT
public:
    explicit FFCaptureUtil(QObject *parent = nullptr);
    ~FFCaptureUtil();

    void initialize();
    void startCapture();
    void stopCapture();

private:
    void initCoreComponents();
    void initCaptureContext();
    void registerMetaTypes();

private:
    FFCapWindow* m_capWindow = nullptr;
    FFCaptureContext* m_captureCtx = nullptr;
    FFThreadPool* m_threadPool = nullptr;
    FFEventLoop* m_eventLoop = nullptr;

    // 解复用器相关
    FFDemuxer* m_vDemuxer1 = nullptr; // 录屏
    FFDemuxer* m_vDemuxer2 = nullptr; // 摄像头
    FFDemuxer* m_vDemuxer3 = nullptr; // 视频文件
    FFDemuxer* m_aDemuxer1 = nullptr; // 声卡
    FFDemuxer* m_aDemuxer2 = nullptr; // 麦克风

    // 队列相关
    class FFAPacketQueue* m_aPktQueue1 = nullptr; // 声卡
    class FFAPacketQueue* m_aPktQueue2 = nullptr; // 麦克风
    class FFVPacketQueue* m_vPktQueue1 = nullptr; // 录屏
    class FFVPacketQueue* m_vPktQueue2 = nullptr; // 摄像头
    class FFVPacketQueue* m_vPktQueue3 = nullptr; // 视频文件
    class FFVFrameQueue* m_vFrmQueue1 = nullptr;  // 录屏
    class FFVFrameQueue* m_vFrmQueue2 = nullptr;  // 摄像头
    class FFVFrameQueue* m_vFrmQueue3 = nullptr;  // 视频文件
    class FFVFrameQueue* m_vRenderFrmQueue = nullptr; // 渲染队列
    class FFAFrameQueue* m_aFrmQueue1 = nullptr;  // 声卡
    class FFAFrameQueue* m_aFrmQueue2 = nullptr;  // 麦克风
    class FFVFrameQueue* m_vFilterEncoderFrmQueue = nullptr; // 视频过滤编码队列
    class FFAFrameQueue* m_aFilterEncoderFrmQueue = nullptr; // 音频过滤编码队列
    class FFAPacketQueue* m_aEncoderPktQueue = nullptr; // 音频编码包队列
    class FFVPacketQueue* m_vEncoderPktQueue = nullptr; // 视频编码包队列

    // 解码器相关
    FFVDecoder* m_vDecoder1 = nullptr; // 录屏
    FFVDecoder* m_vDecoder2 = nullptr; // 摄像头
    FFVDecoder* m_vDecoder3 = nullptr; // 视频文件
    FFADecoder* m_aDecoder1 = nullptr; // 声卡
    FFADecoder* m_aDecoder2 = nullptr; // 麦克风

    // 编码器相关
    FFAEncoder* m_aEncoder = nullptr;
    FFVEncoder* m_vEncoder = nullptr;

    // 过滤器相关
    FFVFilter* m_vFilter = nullptr;
    FFAFilter* m_aFilter = nullptr;

//    // 渲染器相关
//    FFVRender* m_vRender = nullptr;

    // 复用器相关
    FFMuxer* m_muxer = nullptr;

    // 线程相关
    class FFDemuxerThread* m_vDemuxerThread1 = nullptr; // 录屏
    class FFDemuxerThread* m_vDemuxerThread2 = nullptr; // 摄像头
    class FFDemuxerThread* m_vDemuxerThread3 = nullptr; // 视频文件
    class FFDemuxerThread* m_aDemuxerThread1 = nullptr; // 声卡
    class FFDemuxerThread* m_aDemuxerThread2 = nullptr; // 麦克风
    class FFVDecoderThread* m_vDecoderThread1 = nullptr; // 录屏
    class FFVDecoderThread* m_vDecoderThread2 = nullptr; // 摄像头
    class FFVDecoderThread* m_vDecoderThread3 = nullptr; // 视频文件
    class FFADecoderThread* m_aDecoderThread1 = nullptr; // 声卡
    class FFADecoderThread* m_aDecoderThread2 = nullptr; // 麦克风
    class FFAEncoderThread* m_aEncoderThread = nullptr;
    class FFVEncoderThread* m_vEncoderThread = nullptr;
    class FFMuxerThread* m_muxerThread = nullptr;
    class FFVFilterThread* m_vFilterThread = nullptr;
    class FFAFilterThread* m_aFilterThread = nullptr;

    bool m_isCapturing = false;
};

#endif // FFCAPTUREUTIL_H
