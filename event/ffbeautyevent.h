#ifndef FFBEAUTYEVENT_H
#define FFBEAUTYEVENT_H

#include"ffevent.h"
class FFBeautyEvent : public FFEvent
{
public:
    FFBeautyEvent(FFCaptureContext* captureCtx,int smoothValue_,int whiteValue_);

    virtual void work() override;
private:

    int smoothValue;
    int whiteValue;

};

#endif // FFBEAUTYEVENT_H
