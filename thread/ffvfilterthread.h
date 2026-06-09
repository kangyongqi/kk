#ifndef FFVFilterThread_H
#define FFVFilterThread_H

#include"ffthread.h"
#include"opencv/ffoverlayprocessor.h"
extern "C"{
#include<libavutil/time.h>
}
#include<condition_variable>

class FFVFrameQueue;
class FFVFilter;
class FFFaceDetector;
class FFCapWindow;

class FFVFilterThread : public FFThread
{
public:
    FFVFilterThread();
    virtual ~FFVFilterThread() override;

public:
    void init(FFVFrameQueue* frmQueue1_,FFVFrameQueue* frmQueue2_,FFVFrameQueue* frmQueue3_,FFVFrameQueue* renderFrmQueue_,
              FFVFilter* filter_,FFCapWindow* capWindow_);
    void startEncoder();
    void stopEncoder();

    void openVideoSource(int sourceType);
    void closeVideoSource(int sourceType);

    void setWhiteValue(int value);
    void setSmoothValue(int value);

    void pauseEncoder();
    bool peekStart();
    void wakeAllThread();

protected:
    virtual void run() override;

private:
    void overlayFrame(AVFrame* frame,int type);
private:
    FFVFrameQueue* frmQueue1 = nullptr; //screen
    FFVFrameQueue* frmQueue2 = nullptr;  //camera
    FFVFrameQueue* frmQueue3 = nullptr; //video
    FFVFrameQueue* renderFrmQueue = nullptr; //render

    FFVFilter* filter = nullptr;
    FFFaceDetector* faceDetector = nullptr;
    FFCapWindow* capWindow = nullptr;
    FFOverlayProcessor* overlayProcessor = nullptr;

    int overlayX,overlayY;
    int overlayWidth,overlayHeght;

    int64_t overlayPts;
    int64_t overlayDts;

    bool eofFlag = false;

    std::atomic<bool>encoderFlag;
    std::atomic<bool>cameraFlag;
    std::atomic<bool>videoFlag;
    std::atomic<bool>screenFlag;
    AVFrame* screenFrame = nullptr;

    AVFrame* cameraFrame = nullptr;
    AVFrame* cameraFrame2 = nullptr;

    AVFrame* lastVideoFrame = nullptr;
    AVFrame* videoFrame = nullptr;
    AVFrame* videoFrame2 = nullptr;

    std::vector<int>overlayNumbers;

    size_t cameraCount = 0;

    std::mutex mutex;

    std::condition_variable cond;

    int64_t lastVideoTime = 0;
    // 原子类型 ，保证对变量的操作是 不可分割的
    std::atomic<bool>pauseFlag;
    std::atomic<int64_t> pauseTime ;
    std::atomic<int64_t>lastPauseTime;
};

#endif // FFVFilterThread_H
