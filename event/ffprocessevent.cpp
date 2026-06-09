#include "ffprocessevent.h"
#include"player/ffplayercontext.h"


FFProcessEvent::FFProcessEvent(FFPlayerContext *playerCtx, int curSec_)
  :FFEvent (playerCtx),curSec(curSec_)
{

}

FFProcessEvent::~FFProcessEvent()
{

}

void FFProcessEvent::work()
{
    QMetaObject::invokeMethod(playerCtx->playerWindow,"showPlayerProcessSec",Qt::QueuedConnection,Q_ARG(int,curSec));
}
