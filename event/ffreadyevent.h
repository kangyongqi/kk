#ifndef FFREADYEVENT_H
#define FFREADYEVENT_H

#include"ffevent.h"

class FFReadyEvent : public FFEvent
{
public:
    FFReadyEvent(FFPlayerContext* playerCtx,int totalSec_,float aspect_);
    virtual ~FFReadyEvent() override;

    virtual void work()override;

private:
    int totalSec;
    float aspect = 0;
};

#endif // FFREADYEVENT_H
