#ifndef FFOPENVSOURCEEVENT_H
#define FFOPENVSOURCEEVENT_H

#include"ffevent.h"

class FFOpenSourceEvent : public FFEvent
{
public:
    FFOpenSourceEvent(FFCaptureContext* captureCtx,enum FFCaptureContextType::demuxerType sourceType_,
                       std::string const& url_,std::string const& format_);
    virtual void work() override;
    void init();
    void start();

private:
    std::string url;
    std::string format;

    int sourceType;
    int index;
};

#endif // FFOPENVSOURCEEVENT_H
