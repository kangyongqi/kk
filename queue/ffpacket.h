#ifndef FFPACKET_H
#define FFPACKET_H

#endif // FFPACKET_H

extern "C"{
#include"libavformat/avformat.h"
}

enum type{ NORMAL, FLUSH, NULLP };
struct FFPacket
{
    AVPacket packet;
    size_t serial;
    uint8_t type;
};
