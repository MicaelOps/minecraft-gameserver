//
// Created by Micael Cossa on 15/09/2025.
//

#include "packets/EncryptionPacket.h"

void EncryptionPacket::handlePacket(ReadPacketBuffer *packetBuffer, PLAYER_CONNECTION_CONTEXT *connectionContext) {

}

void EncryptionPacket::writeToBuffer(WritePacketBuffer *packetBuffer) {
    packetBuffer->writeVarInt(1);
    packetBuffer->writeString("serverid");
}

void EncryptionPacket::clear() {

}
