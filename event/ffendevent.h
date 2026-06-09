#ifndef FFEndEvent_H
#define FFEndEvent_H

#include"ffevent.h"
class FFEndEvent : public FFEvent
{
public:
    FFEndEvent(FFCaptureContext* captureContext,const std::string audioURL_[FFCaptureContextType::A_DEMUXER_SIZE],
    const std::string videoURL_[FFCaptureContextType::V_DEMUXER_SIZE],
    const std::string audioFMT_[FFCaptureContextType::A_DEMUXER_SIZE],
    const std::string videoFMT_[FFCaptureContextType::V_DEMUXER_SIZE],
    int audioTYPE_[FFCaptureContextType::V_DEMUXER_SIZE],
    int videoTYPE_[FFCaptureContextType::V_DEMUXER_SIZE]);

    virtual ~FFEndEvent() override;

    virtual void work() override;

private:
    void startAll();
    void stopAll();

private:
    size_t demuxerSize;
    size_t aDemuxerSize;
    size_t vDemuxerSize;

    bool audioStartFlags[FFCaptureContextType::A_DEMUXER_SIZE];
    bool videoStartFlags[FFCaptureContextType::V_DEMUXER_SIZE];

    //输入的urls
    std::string audioURL[FFCaptureContextType::A_DEMUXER_SIZE];
    std::string videoURL[FFCaptureContextType::V_DEMUXER_SIZE];

    //输入的formats
    std::string audioFMT[FFCaptureContextType::A_DEMUXER_SIZE];
    std::string videoFMT[FFCaptureContextType::V_DEMUXER_SIZE];

    //解复用类型
    int audioTYPE[FFCaptureContextType::A_DEMUXER_SIZE];
    int videoTYPE[FFCaptureContextType::V_DEMUXER_SIZE];

    //音频解码线程：【声卡】、【麦克风】
    FFADecoderThread* aDecoderThread[FFCaptureContextType::A_DECODER_SIZE];

    //视频解码线程：【屏幕】、【摄像头】、【视频】
    FFVDecoderThread* vDecoderThread[FFCaptureContextType::V_DECODER_SIZE];

    //音频解复用线程：【声卡】、【麦克风】
    FFDemuxerThread* aDemuxerThread[FFCaptureContextType::A_DEMUXER_SIZE];

    //视频解复用器：【屏幕】、【摄像头】、【视频】
    FFDemuxerThread* vDemuxerThread[FFCaptureContextType::V_DEMUXER_SIZE];

    //复用线程
    FFMuxerThread* muxerThread;

    //音频编码线程
    FFAEncoderThread* aEncoderThread;

    //视频编码线程
    FFVEncoderThread* vEncoderThread;

    //视频过滤线程
    FFVFilterThread* vFilterThread;

    //音频滤镜线程
    FFAFilterThread* aFilterThread;
};

#endif // FFEndEvent_H
