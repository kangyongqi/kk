#include "ffvolumeevent.h"

FFVolumeEvent::FFVolumeEvent(FFCaptureContext *captureCtx, double volume_,enum demuxerType sourceType_)
    :FFEvent(captureCtx)
{
    volume = volume_;
    sourceType = sourceType_;
}

void FFVolumeEvent::work()
{
    if(sourceType == AUDIO){
        aFilterThread->setAudioVolume(volume);
    }
    else if(sourceType == MICROPHONE){
        aFilterThread->setMicrophoneVolume(volume);
    }
}
