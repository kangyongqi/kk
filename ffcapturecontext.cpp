#include "ffcapturecontext.h"

FFCaptureContext::FFCaptureContext() {
    // 音频解码线程数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DECODER_SIZE; ++i) {
        aDecoderThread[i] = nullptr;
    }

    // 视频解码线程数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DECODER_SIZE; ++i) {
        vDecoderThread[i] = nullptr;
    }

    // 音频解复用线程数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DEMUXER_SIZE; ++i) {
        aDemuxerThread[i] = nullptr;
    }

    // 视频解复用线程数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DEMUXER_SIZE; ++i) {
        vDemuxerThread[i] = nullptr;
    }

    // 音频解码 packet 队列数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DECODER_SIZE; ++i) {
        aDecoderPktQueue[i] = nullptr;
    }

    // 视频解码 packet 队列数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DECODER_SIZE; ++i) {
        vDecoderPktQueue[i] = nullptr;
    }

    // 视频/音频编码 packet 队列初始化
    vEncoderPktQueue = nullptr;
    aEncoderPktQueue = nullptr;

    //视频/音频filter编码Frame初始化
    vFilterEncoderFrmQueue = nullptr;
    aFilterEncoderFrmQueue = nullptr;


    // 音频解码帧队列数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DECODER_SIZE; ++i) {
        aDecoderFrmQueue[i] = nullptr;
    }

    // 视频解码帧队列数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DECODER_SIZE; ++i) {
        vDecoderFrmQueue[i] = nullptr;
    }


    // 视频渲染帧队列初始化
    vRenderFrmQueue = nullptr;

    //音频解复用器数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DEMUXER_SIZE; ++i) {
        aDemuxer[i] = nullptr;
    }

    //视频解复用器数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DEMUXER_SIZE; ++i) {
        vDemuxer[i] = nullptr;
    }


    // 音频解码器数组初始化
    for (size_t i = 0; i < FFCaptureContextType::A_DECODER_SIZE; ++i) {
        aDecoder[i] = nullptr;
    }

    // 视频解码器数组初始化
    for (size_t i = 0; i < FFCaptureContextType::V_DECODER_SIZE; ++i) {
        vDecoder[i] = nullptr;
    }


//    vRender = nullptr;
    muxer = nullptr;
    muxerThread = nullptr;
    aEncoder = nullptr;
    aEncoderThread = nullptr;
    vEncoder = nullptr;
    vEncoderThread = nullptr;
    vFilter = nullptr;
    vFilterThread = nullptr;
    aFilter = nullptr;
    aFilterThread = nullptr;
    capWindow = nullptr;
}

FFCaptureContext::~FFCaptureContext() {

}
