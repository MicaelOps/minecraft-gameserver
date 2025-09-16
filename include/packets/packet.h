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

    virtual void writeToBuffer(WritePacketBuffer* packetBuffer);

    virtual void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext);
};


#endif //MINECRAFTSERVER_PACKET_H
