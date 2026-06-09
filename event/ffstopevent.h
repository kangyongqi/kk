#ifndef FFSTOPEVENT_H
#define FFSTOPEVENT_H

#include"ffevent.h"

class FFStopEvent : public FFEvent
{
public:
    FFStopEvent(FFCaptureContext* captureCtx);

    virtual void work()override;

private:
    void close();
    void clearQueue();

private:
    std::string url;
    std::string format;
};



#endif // FFSTOPEVENT_H
