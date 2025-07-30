//
// Created by Micael Cossa on 26/07/2025.
//


#include "server_handler.h"
#include "server_utils.h"
#include "packets/HandshakePacket.h"




//constexpr std::unordered_map<int, Packet> packet_handler;
//constexpr std::unordered_set<player> players;

Packet* (*packetFactory[256])() = {nullptr};

void setupPacketFactory() {
    packetFactory[0] = [] () -> Packet* { return new HandshakePacket(); };
}

void invokePacket(PacketBuffer* packetBuffer, QUEUED_CONNECTION_CONTEXT* connectionContext) {

    int packetId = packetBuffer->readVarInt();

    if(packetId > 255 || packetId < 0) {
        printInfo("invalid Packet id: ", packetId);
        return;
    }

    Packet* packet = packetFactory[packetId]();
    packet->readFromBuffer(packetBuffer);
    packet->handlePacket(connectionContext);

    delete packet;
}

/**
 * Packet format
 * - length (id + data)
 * - id
 * - data
 * @param packet
 * @param connectionContext
 */
void sendPacket(Packet* packet, QUEUED_CONNECTION_CONTEXT* connectionContext) {

    PacketBuffer packetBuffer;
    packet->writeToBuffer(&packetBuffer);
    packetBuffer.readjustBufferSize();
    packetBuffer.writeVarInt(packetBuffer.bufferSize);
    sendDataToConnection(&packetBuffer, connectionContext);
}
