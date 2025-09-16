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

    void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) override;

    void writeToBuffer(WritePacketBuffer* packetBuffer) override;

    void clear() override;

    HandshakePacket() {
        protocolVersion =-1;
        port = -1;
        state = -1;
    }
};


#endif //MINECRAFTSERVER_HANDSHAKEPACKET_H
