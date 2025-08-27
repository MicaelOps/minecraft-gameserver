//
// Created by Micael Cossa on 26/07/2025.
//

#include <unordered_map>
#include <map>
#include <memory>
#include <utility>
#include "server_handler.h"
#include "server_utils.h"
#include "server_connection.h"
#include "packets/HandshakePacket.h"
#include "packets/PingPongPacket.h"
#include "packets/UndefinedPacket.h"
#include "packets/ServerQueryPacket.h"


using PacketGenerator = std::unique_ptr<Packet>(*)();

std::array<PacketGenerator, 256> packetFactory;

void setupPacketFactory() {

    packetFactory.fill([] () -> std::unique_ptr<Packet> { return std::make_unique<UndefinedPacket>(); });

    packetFactory[std::to_underlying(ConnectionState::HANDSHAKING) + 0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<HandshakePacket>(); };

    packetFactory[std::to_underlying(ConnectionState::STATUS) + 0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<ServerQueryPacket>(); };
    packetFactory[std::to_underlying(ConnectionState::STATUS) + 1] = [] () -> std::unique_ptr<Packet> { return std::make_unique<PingPongPacket>(); };
}

void invokePacket(ReadPacketBuffer* packetBuffer, CONNECTION_INFO* connectionInfo) {


    int packetId = packetBuffer->readVarInt();

    printInfo("Received a packet with id: " , packetId);

    if(packetId > 255 || packetId < 0) {
        printInfo("invalid Packet id: ", packetId);
        return;
    }

    std::unique_ptr<Packet> packet = packetFactory[std::to_underlying(connectionInfo->connectionState) + packetId]();
    packet->readFromBuffer(packetBuffer);
    packet->handlePacket(connectionInfo);

}

/**
 * Packet format
 * - length (id + data)
 * - id
 * - data
 * @param packet
 * @param connectionContext
 */
void sendPacket(Packet* packet, const CONNECTION_INFO* connectionInfo) {


    std::unique_ptr<WritePacketBuffer> packetBuffer = std::make_unique<WritePacketBuffer>();

    // writes packet data + packet id
    packet->writeToBuffer(packetBuffer.get());

    // packet size should be the first varInt to be read
    packetBuffer->writeVarIntAttheBack(packetBuffer->getSize());

    // dummy byte, for some reason minecraft drops one byte (which would completely mess up the way it was read) despite WSASend confirming the correct amount of bytes sent
    // The fact that I am writing this,  despite knowing this project wont be shared, it would give insights to the levels of frustrations.
    // But if you are not the future me and is someone who just happened to come across this file, this single line of code contains days of agony
    packetBuffer->writeByte(0);


    sendDataToConnection(packetBuffer.get(), connectionInfo);

}
