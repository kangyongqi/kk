#include "ffreadyevent.h"

FFReadyEvent::FFReadyEvent(FFPlayerContext *playerCtx, int totalSec_,float aspect_)
    :FFEvent (playerCtx),totalSec(totalSec_),aspect(aspect_)
{

}

FFReadyEvent::~FFReadyEvent()
{

}

void FFReadyEvent::work()
{
    QMetaObject::invokeMethod(playerCtx->playerWindow,"initPlayerTotalSec",Qt::QueuedConnection,Q_ARG(int,totalSec),Q_ARG(float,aspect));
}
