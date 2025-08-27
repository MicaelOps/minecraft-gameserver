//
// Created by Micael Cossa on 27/08/2025.
//

#ifndef CORE_MINESERVER_SERVERQUERYPACKET_H
#define CORE_MINESERVER_SERVERQUERYPACKET_H
#include "packet.h"

class ServerQueryPacket: public Packet {

public:
    void readFromBuffer(ReadPacketBuffer* packetBuffer) override {};

    void handlePacket(CONNECTION_INFO* connectionInfo) override {};

    void writeToBuffer(WritePacketBuffer* packetBuffer) override {};


    ServerQueryPacket() = default;
};


#endif //CORE_MINESERVER_SERVERQUERYPACKET_H
