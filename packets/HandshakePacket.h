//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_HANDSHAKEPACKET_H
#define MINECRAFTSERVER_HANDSHAKEPACKET_H

#include <string>
#include "packet.h"
class HandshakePacket : public Packet {

private:
    int protocolVersion;
    std::string serverAddress, response;
    unsigned short port;
    int state;

public:

    void readFromBuffer(PacketBuffer* packetBuffer) override;

    void handlePacket(SOCKET playerSocket) override;

    void writeToBuffer(PacketBuffer* packetBuffer) override;


    HandshakePacket() {
        packetType = SERVER_PACKET;
        protocolVersion =-1;
        port = -1;
        state = -1;
    }
};


#endif //MINECRAFTSERVER_HANDSHAKEPACKET_H
