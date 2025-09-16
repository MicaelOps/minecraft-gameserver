//
// Created by Micael Cossa on 26/07/2025.
//

#include <map>
#include <memory>
#include <utility>
#include <chrono>
#include "server_connection.h"
#include "packet_handler.h"
#include "server_utils.h"
#include "packets/HandshakePacket.h"
#include "packets/PingPongPacket.h"
#include "packets/UndefinedPacket.h"
#include "packets/ServerQueryPacket.h"
#include "packets/LoginStartPacket.h"
#include "packets/EncryptionPacket.h"


using PacketGenerator = std::unique_ptr<Packet>(*)();

namespace  {
    // Minecraft packets dont exceed 256
    std::array<PacketGenerator, 256> packetFactory;
}


void setupPacketFactory() {

    packetFactory.fill([] () -> std::unique_ptr<Packet> { return std::make_unique<UndefinedPacket>(); });

    packetFactory[std::to_underlying(ConnectionState::HANDSHAKING) + 0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<HandshakePacket>(); };

    packetFactory[std::to_underlying(ConnectionState::STATUS) + 0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<ServerQueryPacket>(); };
    packetFactory[std::to_underlying(ConnectionState::STATUS) + 1] = [] () -> std::unique_ptr<Packet> { return std::make_unique<PingPongPacket>(); };

    packetFactory[std::to_underlying(ConnectionState::LOGIN) + 0] = [] () -> std::unique_ptr<Packet> { return std::make_unique<LoginStartPacket>(); };
    packetFactory[std::to_underlying(ConnectionState::LOGIN) + 1] = [] () -> std::unique_ptr<Packet> { return std::make_unique<EncryptionPacket>(); };
}

void invokePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) {

    int packetId = packetBuffer->readVarInt();

    if(packetId > 255 || packetId < 0) {
        printInfo("invalid Packet id:  ", packetId);
        return;
    }

    std::unique_ptr<Packet> packet = packetFactory[std::to_underlying(connectionContext->connectionInfo.connectionState) + packetId]();

    packet->handlePacket(packetBuffer, connectionContext);

}

/**
 * Packet format
 * - length (id + data)
 * - id
 * - data
 * @param packet
 * @param connectionContext
 */
void sendPacket(Packet* packet, PLAYER_CONNECTION_CONTEXT* connectionContext) {

    PLAYER_CONNECTION_CONTEXT* sendcontext = borrowContext();

    if (!sendcontext) {
        printInfo("Failed to acquire context for sending");
        return;
    }

    // copy the socket, state, status
    sendcontext->copy(connectionContext);

    std::unique_ptr<WritePacketBuffer> packetBuffer = std::make_unique<WritePacketBuffer>(sendcontext->buffer.buf, sendcontext->buffer.len);

    packet->writeToBuffer(packetBuffer.get());

    packetBuffer->writeVarIntAtTheFront((int)packetBuffer->getSize());


    // dummy byte, for some reason minecraft drops one byte (which would completely mess up the way it was read) despite WSASend confirming the correct amount of bytes sent
    // The fact that I am writing this,  despite knowing this project wont be shared, it would give insights to the levels of frustrations.
    // But if you are not the future me and is someone who just happened to come across this file, this single line of code contains days of agony
    packetBuffer->writeByte(0);


    sendDataToConnection(sendcontext);

}
