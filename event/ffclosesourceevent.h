#ifndef FFCLOSESOURCEEVENT_H
#define FFCLOSESOURCEEVENT_H

#include"ffevent.h"

class FFCloseSourceEvent : public FFEvent
{
public:
    FFCloseSourceEvent(FFCaptureContext* captureCtx,enum FFCaptureContextType::demuxerType sourceType_);

    virtual void work() override;

private:
    void close();
private:

    int sourceType;
    int index;
};

#endif // FFCLOSESOURCEEVENT_H
