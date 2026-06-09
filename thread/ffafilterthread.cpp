#include "ffafilterthread.h"
#include"queue/ffaframequeue.h"
#include"filter/ffafilter.h"
#include"capture/ffcapturecontext.h"
#include"clock/ffglobalclock.h"

using namespace FFCaptureContextType;
FFAFilterThread::FFAFilterThread()
{
    encoderFlag.store(false);
    audioFlag.store(false);
    microphoneFlag.store(false);
    pauseFlag.store(false);

    pauseTime = 0;
    lastPauseTime = 0;
}

FFAFilterThread::~FFAFilterThread()
{
    if(audioFrame){
        av_frame_free(&audioFrame);
    }
    if(microphoneFrame){
        av_frame_free(&microphoneFrame);
    }
}

void FFAFilterThread::openAudioSource(int audioType)
{
//    cond.notify_all();
    std::lock_guard<std::mutex> lock(mutex);
    enum demuxerType type = static_cast<demuxerType>(audioType);
    if(type == AUDIO){
        audioFlag.store(true,std::memory_order_seq_cst);
    }
    else if(type == MICROPHONE){
        microphoneFlag.store(true,std::memory_order_seq_cst);
    }
    cond.notify_one();

}

void FFAFilterThread::closeAudioSource(int audioType)
{
    cond.notify_all();
    enum demuxerType type = static_cast<demuxerType>(audioType);
    if(type == AUDIO){
        audioFlag.store(false,std::memory_order_seq_cst);
    }
    else if(type == MICROPHONE){
        microphoneFlag.store(false,std::memory_order_seq_cst);
    }

}

void FFAFilterThread::init(FFAFrameQueue *frmQueue1_, FFAFrameQueue* frmQueue2_, FFAFilter *filter_)
{
    frmQueue1 = frmQueue1_;
    frmQueue2 = frmQueue2_;
    filter = filter_;
}

void FFAFilterThread::startEncoder()
{
    cond.notify_all();
    // 路口A的灯变绿 → 路口B的灯变绿 → 路口C的灯变绿
    // 所有司机都看到这个顺序 ，没有例外！
    // 最严格，最安全
    encoderFlag.store(true,std::memory_order_seq_cst);
}

void FFAFilterThread::stopEncoder()
{
    cond.notify_all();
    encoderFlag.store(false,std::memory_order_seq_cst);
    pauseFlag.store(false);
    pauseTime = 0;
    lastPauseTime = 0;
}

void FFAFilterThread::pauseEncoder()
{

    std::lock_guard<std::mutex>lock(mutex);
    if(pauseFlag.load()){
        pauseTime +=  av_gettime_relative() - lastPauseTime;
        pauseFlag.store(false);
    }
    else{
        pauseFlag.store(true);
        lastPauseTime = av_gettime_relative();
    }
}

void FFAFilterThread::setAudioVolume(double value)
{
    if(filter){
        if(audioFlag.load() && microphoneFlag.load()){
            filter->setVolume(value,-1);
        }
        else{
            filter->setAudioVolume(value);
        }
    }
}

void FFAFilterThread::setMicrophoneVolume(double value)
{
    if(filter){
        if(audioFlag.load() && microphoneFlag.load()){
            filter->setVolume(-1,value);
        }
        else{
            filter->setMicrophoneVolume(value);
        }
    }
}

bool FFAFilterThread::peekStart()
{
    return encoderFlag.load();
}

void FFAFilterThread::wakeAllThread()
{
    if(frmQueue1){
        frmQueue1->wakeAllThread();
    }
    if(frmQueue2){
        frmQueue2->wakeAllThread();
    }
}

void FFAFilterThread::run()
{
    while(!m_stop){
        bool microphoneActive = microphoneFlag.load(std::memory_order_seq_cst);
        bool audioActive = audioFlag.load(std::memory_order_seq_cst);
        bool encoderActive = encoderFlag.load(std::memory_order_seq_cst);
        bool pauseActive = pauseFlag.load(std::memory_order_seq_cst);

        //        std::cout << "pause :" <<pauseActive << std::endl;
        if(!microphoneActive && !audioActive && !encoderActive){
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait_for(lock,std::chrono::milliseconds(100));
            continue; // 重新读取所有标志
        }

        int64_t start = av_gettime_relative() * 10;

        // 混音：【声卡】 + 【麦克风】
        if(audioActive && microphoneActive){
            audioFrame = frmQueue1->dequeue();
            microphoneFrame = frmQueue2->dequeue();

            if(encoderActive && !pauseActive){
                int ret = filter->sendFilter(audioFrame, microphoneFrame, start,pauseTime);
                if(ret < 0){
                    std::cerr << "FilterThread Fail !" << std::endl;
                    m_stop = true;
                }
            }
            else{
                if(audioFrame){
                    av_frame_unref(audioFrame);
                    av_frame_free(&audioFrame);
                }
                if(microphoneFrame){
                    av_frame_unref(microphoneFrame);
                    av_frame_free(&microphoneFrame);
                }
            }
        }
        // 【声卡】
        else if(audioActive){
            audioFrame = frmQueue1->dequeue();
            if(encoderActive && !pauseActive){
                //                filter->sendFrame(audioFrame, start);
                filter->sendSingleFilter(audioFrame, start,pauseTime,AUDIO);
            }
            else{
                if(audioFrame){
                    av_frame_unref(audioFrame);
                    av_frame_free(&audioFrame);
                }
            }
        }
        // 【麦克风】
        else if(microphoneActive){
            microphoneFrame = frmQueue2->dequeue();
            if(encoderActive && !pauseActive){
                //                filter->sendFrame(microphoneFrame, start);
                filter->sendSingleFilter(microphoneFrame, start,pauseTime,MICROPHONE);
            }
            else{
                if(microphoneFrame){
                    av_frame_unref(microphoneFrame);
                    av_frame_free(&microphoneFrame);
                }
            }
        }
        // 静音包
        else{
            if(encoderActive && !pauseActive){
                AVFrame* muteFrame = generateMuteFrame();
                filter->sendFrame(muteFrame, start,pauseTime);
            }
        }
    }
}

AVFrame *FFAFilterThread::generateMuteFrame()
{
    AVFrame* frame = av_frame_alloc();

    frame->format = FF_SAMPLE_FMT;
    frame->sample_rate = FF_SAMPLE_RATE;
    frame->nb_samples = 1024;
    av_channel_layout_default(&frame->ch_layout,2);


    int ret = av_frame_get_buffer(frame,0);
    if(ret < 0){
        std:: cerr <<"get mute Frame buffer error" << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }

    ret = av_samples_set_silence(frame->extended_data,0,frame->nb_samples,frame->ch_layout.nb_channels,FF_SAMPLE_FMT);
    if(ret < 0){
        std::cerr << "Set Samples Silence Fail !" << std::endl;
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}
