//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_PACKET_H
#define MINECRAFTSERVER_PACKET_H

#include "server_connection.h"
#include "packet_buf.h"



class Packet {

public:


    virtual ~Packet() = default;

    virtual void readFromBuffer(ReadPacketBuffer* packetBuffer);

    virtual void writeToBuffer(WritePacketBuffer* packetBuffer);

    virtual void handlePacket(PLAYER_CONNECTION_CONTEXT* connectionContext);
};


#endif //MINECRAFTSERVER_PACKET_H
