#ifndef FFSPEEDEVENT_H
#define FFSPEEDEVENT_H

#include"ffevent.h"

class FFSpeedEvent : public FFEvent
{
public:
    FFSpeedEvent(FFPlayerContext* playerCtx,float speed_);

    virtual void work();
private:
    float speed;
};

#endif // FFSPEEDEVENT_H
