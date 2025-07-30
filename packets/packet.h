//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_PACKET_H
#define MINECRAFTSERVER_PACKET_H

#define PACKET_ID_SIZE char

#include "../packet_buf.h"
#include "../server_connection.h"


enum PacketType {
    WORLD_PACKET, SERVER_PACKET
};

class Packet {

public:

    PacketType packetType = SERVER_PACKET;

    virtual ~Packet() = default;

    virtual void readFromBuffer(PacketBuffer* packetBuffer);

    virtual void writeToBuffer(PacketBuffer* packetBuffer);

    virtual void handlePacket(QUEUED_CONNECTION_CONTEXT* connectionContext);
};


#endif //MINECRAFTSERVER_PACKET_H
