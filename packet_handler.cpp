//
// Created by Micael Cossa on 26/07/2025.
//

#include <map>
#include <memory>
#include <utility>
#include <chrono>
#include "packet_handler.h"

#include "minecraft.h"

#include "packets/HandshakePacket.h"
#include "packets/PingPongPacket.h"
#include "packets/UndefinedPacket.h"
#include "packets/ServerQueryPacket.h"
#include "packets/LoginStartPacket.h"
#include "packets/EncryptionPacket.h"


namespace  {
    thread_local std::array<std::unique_ptr<Packet>, 256> packetFactory = {};
    constexpr auto handshake_index = std::to_underlying(ConnectionState::HANDSHAKING);
    constexpr auto status_index = std::to_underlying(ConnectionState::STATUS);
    constexpr auto login_index = std::to_underlying(ConnectionState::LOGIN);
    constexpr auto play_index = std::to_underlying(ConnectionState::PLAY);
    constexpr auto disconnect_index = std::to_underlying(ConnectionState::DISCONNECT);
}
void setupPacketFactory() {

    for (auto& packet : packetFactory) {
        packet = std::make_unique<UndefinedPacket>();
    }

    packetFactory[std::to_underlying(ConnectionState::HANDSHAKING) + 0] = std::make_unique<HandshakePacket>();
    packetFactory[std::to_underlying(ConnectionState::STATUS) + 0] = std::make_unique<ServerQueryPacket>();
    packetFactory[std::to_underlying(ConnectionState::STATUS) + 1] = std::make_unique<PingPongPacket>();
    packetFactory[std::to_underlying(ConnectionState::LOGIN) + 0] = std::make_unique<LoginStartPacket>();
    packetFactory[std::to_underlying(ConnectionState::LOGIN) + 1] = std::make_unique<EncryptionPacket>();
}


bool isPacketIdInBounds(int playerState, int packetid) {

    switch (playerState) {
        case handshake_index:
            return packetid >= handshake_index && packetid < status_index;
        case status_index:
            return packetid >= status_index && packetid < login_index;
        case login_index:
            return packetid >= login_index && packetid < play_index;
        case play_index:
            return packetid >= play_index && packetid < disconnect_index;
        default:
            return false; // not unreachable, (logically it is)
    }
}

//RECEIVE_DATA_EVENT_HANDLER method from NetworkManager
void invokePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) {

    int packetId = packetBuffer->readVarInt();

    if(packetId >= packetFactory.size() || packetId < 0 ) {
        printInfo("invalid Packet id:  ", packetId);
        return;
    }

    int stateNum = std::to_underlying(connectionContext->connectionInfo.connectionState);

    // Checking if the player is invoking a packet within the bounds of its connection state
    // Example: A player pinging the server should not sending a EncryptionPacket.

    if(!isPacketIdInBounds(stateNum, packetId))
        return;



    packetFactory[stateNum + packetId]->handlePacket(packetBuffer, connectionContext);
    packetFactory[stateNum + packetId]->clear();

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

    PLAYER_CONNECTION_CONTEXT* sendcontext = Minecraft::getNetworkManager().acquireContext();

    if (!sendcontext) {
        printInfo("Failed to acquire context for sending");
        return;
    }

    // copy the socket, state, status
    sendcontext->copy(connectionContext);

    WritePacketBuffer packetBuffer = WritePacketBuffer(sendcontext->buffer.buf, sendcontext->buffer.len);

    packet->writeToBuffer(&packetBuffer);

    packetBuffer.writeVarIntAtTheFront((int)packetBuffer.getSize());


    // dummy byte, for some reason minecraft drops one byte (which would completely mess up the way it was read) despite WSASend confirming the correct amount of bytes sent
    // The fact that I am writing this,  despite knowing this project wont be shared, it would give insights to the levels of frustrations.
    // But if you are not the future me and is someone who just happened to come across this file, this single line of code contains days of agony
    packetBuffer.writeByte(0);

    Minecraft::getNetworkManager().sendDataToConnection(sendcontext);

}
