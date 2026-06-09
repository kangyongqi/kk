#include "ffpauseevent.h"

FFPauseEvent::FFPauseEvent(FFCaptureContext* captureCtx)
    :FFEvent (captureCtx)
{

}

FFPauseEvent::~FFPauseEvent()
{

}

void FFPauseEvent::work()
{
    vFilterThread->pauseEncoder();
    aFilterThread->pauseEncoder();
}


