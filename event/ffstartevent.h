#ifndef FFSTARTEVENT_H
#define FFSTARTEVENT_H

#include "ffevent.h"

using namespace FFCaptureContextType;
class FFStartEvent : public FFEvent
{
public:
    FFStartEvent(FFCaptureContext* captureCtx,std::string const&url_,std::string const& format_);

    virtual void work()override;

private:
    void initEncoder();
    void startQueue();
    void startEncoder();
private:

    std::string url;
    std::string format;
};

#endif // FFSTARTEVENT_H
