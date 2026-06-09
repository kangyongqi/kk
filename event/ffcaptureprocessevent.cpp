#include "ffcaptureprocessevent.h"

FFCaptureProcessEvent::FFCaptureProcessEvent(FFCaptureContext *captureCtx,int64_t millseconds_)
    :FFEvent (captureCtx)
{
    millseconds = millseconds_;
}

void FFCaptureProcessEvent::work()
{
    QMetaObject::invokeMethod(capWindow,"setCaptureProcessTime",Qt::QueuedConnection,Q_ARG(int64_t,millseconds));
}


