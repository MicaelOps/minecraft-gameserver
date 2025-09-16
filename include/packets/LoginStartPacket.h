//
// Created by Micael Cossa on 26/08/2025.
//

#ifndef CORE_MINESERVER_LOGINSTARTPACKET_H
#define CORE_MINESERVER_LOGINSTARTPACKET_H

#include "packet.h"
/*
 * Both ping and pong requests are handled here.
 *
 * Client sends a Ping request after the HandshakePacket is successfully handled.
 * Server responds with a pong.
 */

class LoginStartPacket : public Packet {

private:
    std::string name, disconnectReason;

public:

    void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) override;

    void writeToBuffer(WritePacketBuffer* packetBuffer) override;

    void clear() override;

    LoginStartPacket() = default;
};


#endif //CORE_MINESERVER_LOGINSTARTPACKET_H
