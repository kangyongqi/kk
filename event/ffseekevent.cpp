#include "ffseekevent.h"
#include"globalclock.h"
FFSeekEvent::FFSeekEvent(FFPlayerContext *playerCtx, int64_t seekSec_)
    :FFEvent (playerCtx),seekSec(seekSec_)
{

}

void FFSeekEvent::work()
{
    playerCtx->demuxerThread->seek(seekSec);
//    playerCtx->aRender->seek();
//    playerCtx->vRender->seek();
//    globalClock::getInstance()->setClock(seekSec * 1000);
//    std::cerr<<"seek clock:"<<seekSec * 1000<<std::endl;
}
