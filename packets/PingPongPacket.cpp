//
// Created by Micael Cossa on 26/08/2025.
//

#include <iostream>
#include "packets/PingPongPacket.h"
#include "packet_handler.h"

void PingPongPacket::readFromBuffer(ReadPacketBuffer *packetBuffer) {

    ping = packetBuffer->readLong();

}

void PingPongPacket::handlePacket(PLAYER_CONNECTION_CONTEXT* playerConnectionContext) {

    sendPacket(this, playerConnectionContext);

    closeConnection(playerConnectionContext->connectionInfo.playerSocket); // Status Request is over
}

void PingPongPacket::writeToBuffer(WritePacketBuffer *packetBuffer) {

    packetBuffer->writeVarInt(1); // packet id
    packetBuffer->writeLong(ping);

}
