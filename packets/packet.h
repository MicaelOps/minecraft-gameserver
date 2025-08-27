//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_PACKET_H
#define MINECRAFTSERVER_PACKET_H

#include "../packet_buf.h"
#include "../server_connection.h"



class Packet {

public:


    virtual ~Packet() = default;

    virtual void readFromBuffer(ReadPacketBuffer* packetBuffer);

    virtual void writeToBuffer(WritePacketBuffer* packetBuffer);

    virtual void handlePacket(CONNECTION_INFO* connectionInfo);
};


#endif //MINECRAFTSERVER_PACKET_H
