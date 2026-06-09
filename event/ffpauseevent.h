#ifndef FFPAUSEEVENT_H
#define FFPAUSEEVENT_H

#include"ffevent.h"

class FFPauseEvent : public FFEvent
{
public:
    FFPauseEvent(FFCaptureContext* captureCtx);
    virtual ~FFPauseEvent()override;

    virtual void work() override;


};

#endif // FFPAUSEEVENT_H
