//
// Created by Micael Cossa on 26/07/2025.
//

#include <memory>
#include "server_handler.h"
#include "server_utils.h"
#include "server_connection.h"
#include "packets/HandshakePacket.h"




// std::unordered_map<int, Packet> packet_handler;
// std::unordered_set<player> players;

std::unique_ptr<Packet> (*packetFactory[256])() = {nullptr};

void setupPacketFactory() {
    packetFactory[0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<HandshakePacket>(); };
}

void invokePacket(PacketBuffer* packetBuffer, SOCKET playerSocket) {

    int packetId = packetBuffer->readVarInt();

    if(packetId > 255 || packetId < 0) {
        printInfo("invalid Packet id: ", packetId);
        return;
    }

    std::unique_ptr<Packet> packet = packetFactory[packetId]();
    packet->readFromBuffer(packetBuffer);
    packet->handlePacket(playerSocket);

}

/**
 * Packet format
 * - length (id + data)
 * - id
 * - data
 * @param packet
 * @param connectionContext
 */
void sendPacket(Packet* packet, SOCKET playerSocket) {

    std::unique_ptr<PacketBuffer> packetBuffer = std::make_unique<VectorBuffer>();

    printInfo("----- ");
    // writes packet data + packet id
    packet->writeToBuffer(packetBuffer.get());
    printInfo("----- ");
    printInfo("Size of the packet: ", packetBuffer->getSize());

    // packet size should be the first varInt to be read
    packetBuffer->writeVarIntAttheBack(packetBuffer->getSize());

    // dummy byte, for some reason minecraft drops one byte (which would completely mess up the way it was read) despite WSASend confirming the correct amount of bytes sent
    // The fact that I am writing this,  despite knowing this project wont be shared, it would provide insight to the levels of frustrations.
    // But if you are not the future me and is someone who just happened to come across this file, this line code is worth an ungodly amount of time.
    packetBuffer->writeByte(0);


    sendDataToConnection(packetBuffer.get(), playerSocket);

}
