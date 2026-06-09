#ifndef FFVOLUMEEVENT_H
#define FFVOLUMEEVENT_H

#include"ffevent.h"
class FFVolumeEvent : public FFEvent
{
public:
    FFVolumeEvent(FFCaptureContext* playerCtx,double volume_,enum demuxerType sourceType_);

    virtual void work()override;

private:
    double volume;
    enum demuxerType sourceType;
};

#endif // FFVOLUMEEVENT_H
