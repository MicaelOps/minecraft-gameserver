//
// Created by Micael Cossa on 26/08/2025.
//

#ifndef CORE_MINESERVER_PINGPONGPACKET_H
#define CORE_MINESERVER_PINGPONGPACKET_H

#include "packet.h"
/*
 * Both ping and pong requests are handled here.
 *
 * Client sends a Ping request after the HandshakePacket is successfully handled.
 * Server responds with a pong.
 */

class PingPongPacket : public Packet {

private:
    long long int ping = 0; // 64-bit integer

public:
    void readFromBuffer(ReadPacketBuffer* packetBuffer) override;

    void handlePacket(PLAYER_CONNECTION_CONTEXT* connectionContext) override;

    void writeToBuffer(WritePacketBuffer* packetBuffer) override;


    PingPongPacket() = default;
};


#endif //CORE_MINESERVER_PINGPONGPACKET_H
