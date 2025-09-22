//
// Created by Micael Cossa on 14/09/2025.
//

#include "packets/LoginStartPacket.h"
#include "minecraft.h"
#include "../minecraft_internal.h"
#include "packet_handler.h"

// LoginStartPacket acts as a Disconnect Packet and Login Start because they have the same Packet ID

void LoginStartPacket::handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* playerConnectionContext) {
    name = packetBuffer->readString();

    if(Minecraft::playerExistsByName(name)) {
        disconnectReason = std::string("'").append(std::string(getColour(COLOUR::RED)).append("Player already exists'"));
        sendPacket(this, playerConnectionContext);
        getNetworkManager().closeConnection(playerConnectionContext);
    }

}

void LoginStartPacket::writeToBuffer(WritePacketBuffer *packetBuffer) {
    packetBuffer->writeVarInt(0);
    packetBuffer->writeString(disconnectReason);
}

void LoginStartPacket::clear() {

}
