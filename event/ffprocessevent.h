#ifndef FFPROCESSEVENT_H
#define FFPROCESSEVENT_H

#include"ffevent.h"

class FFPlayerWindow;

class FFProcessEvent : public FFEvent
{
public:
    FFProcessEvent(FFPlayerContext* playerCtx,int curSec_);
    virtual ~FFProcessEvent()override;

    virtual void work()override;

private:
    int curSec;
};

#endif // FFPROCESSEVENT_H
