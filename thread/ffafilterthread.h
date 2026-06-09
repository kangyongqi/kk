#ifndef FFAFilterThread_H
#define FFAFilterThread_H

#include"ffthread.h"
#include<mutex>
#include<condition_variable>


extern "C"{
#include<libavutil/time.h>
}

class FFAFrameQueue;
class FFAFilter;
class AVFrame;

class FFAFilterThread : public FFThread
{
public:
    FFAFilterThread();
    virtual ~FFAFilterThread() override;

    void openAudioSource(int audioType);
    void closeAudioSource(int audioType);
    void init(FFAFrameQueue* frmQueue1_,FFAFrameQueue* frmQueue2_, FFAFilter* filter_);

    void startEncoder();
    void stopEncoder();
    void pauseEncoder();

    void setAudioVolume(double value);
    void setMicrophoneVolume(double value);

    bool peekStart();
    void wakeAllThread();
protected:
    virtual void run() override;

private:
    AVFrame* generateMuteFrame();
private:
    FFAFrameQueue* frmQueue1 = nullptr;
    FFAFrameQueue* frmQueue2 = nullptr;  //camera
    FFAFilter* filter = nullptr;

    AVFrame* audioFrame = nullptr;
    AVFrame* microphoneFrame = nullptr;

    std::atomic<bool>encoderFlag;
    std::atomic<bool>audioFlag;
    std::atomic<bool>microphoneFlag;
    std::atomic<bool>pauseFlag;

    std::mutex mutex;
    std::condition_variable cond;
    std::atomic<int64_t> pauseTime;
    std::atomic<int64_t>  lastPauseTime;
};

#endif // FFAFilterThread_H
