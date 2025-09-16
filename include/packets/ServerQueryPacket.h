//
// Created by Micael Cossa on 27/08/2025.
//

#ifndef CORE_MINESERVER_SERVERQUERYPACKET_H
#define CORE_MINESERVER_SERVERQUERYPACKET_H
#include "packet.h"

class ServerQueryPacket: public Packet {

public:

    void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext)override {};

    void writeToBuffer(WritePacketBuffer* packetBuffer) override {};

    void clear() override {};

    ServerQueryPacket() = default;
};



#endif //CORE_MINESERVER_SERVERQUERYPACKET_H
