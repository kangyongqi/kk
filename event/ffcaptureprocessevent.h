#ifndef FFCAPTUREPROCESSEVENT_H
#define FFCAPTUREPROCESSEVENT_H

#include"ffevent.h"
// 音视频采集流程控制事件类
class FFCaptureProcessEvent : public FFEvent
{
public:
    FFCaptureProcessEvent(FFCaptureContext* captureCtx,int64_t millseconds_);

    virtual void work()override;

private:
    // 定时时长（毫秒）
    int64_t millseconds;
};

#endif // FFCAPTUREPROCESSEVENT_H
