#include "ffspeedevent.h"


FFSpeedEvent::FFSpeedEvent(FFPlayerContext *playerCtx, float speed_)
    :FFEvent (playerCtx),speed(speed_)
{

}

void FFSpeedEvent::work()
{
    playerCtx->aRender->setSpeed(speed);
    playerCtx->vRender->setSpeed(speed);
}
