//
// Created by Micael Cossa on 26/08/2025.
//

#include "PingPongPacket.h"
#include "../server_handler.h"

void PingPongPacket::readFromBuffer(ReadPacketBuffer *packetBuffer) {
    ping = packetBuffer->readLong();
}

void PingPongPacket::handlePacket(CONNECTION_INFO* connectionInfo) {

    sendPacket(this, connectionInfo);

    closeConnection(connectionInfo->playerSocket); // Status Request is over
}

void PingPongPacket::writeToBuffer(WritePacketBuffer *packetBuffer) {
    packetBuffer->writeVarInt(1); // packet id
    packetBuffer->writeLong(ping);
}
