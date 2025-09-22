//
// Created by Micael Cossa on 26/08/2025.
//

#include <chrono>
#include "packets/PingPongPacket.h"
#include "packet_handler.h"
#include "../minecraft_internal.h"


void PingPongPacket::handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* playerConnectionContext) {

    ping = packetBuffer->readLong();
    sendPacket(this, playerConnectionContext);

    getNetworkManager().closeConnection(playerConnectionContext);

}

void PingPongPacket::writeToBuffer(WritePacketBuffer *packetBuffer) {

    packetBuffer->writeVarInt(1); // packet id
    packetBuffer->writeLong(ping);

}

void PingPongPacket::clear() {

}
