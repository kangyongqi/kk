#include "ffbeautyevent.h"


FFBeautyEvent::FFBeautyEvent(FFCaptureContext *captureCtx, int smoothValue_, int whiteValue_)
    :FFEvent (captureCtx)
{
    smoothValue = smoothValue_;
    whiteValue = whiteValue_;
}

void FFBeautyEvent::work()
{
    vFilterThread->setWhiteValue(whiteValue);
    vFilterThread->setSmoothValue(smoothValue);
}
