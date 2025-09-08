//
// Created by Micael Cossa on 07/08/2025.
//

#ifndef MINECRAFTSERVER_SERVER_CONNECTION_H
#define MINECRAFTSERVER_SERVER_CONNECTION_H

// Linux implementation will come later, maybe when I have a spare laptop or this project is near complete.
#include <winsock2.h>
#include <memory>
#include "concurrent_unordered_set.h"
#include "packet_buf.h"

#pragma comment(lib, "Ws2_32.lib")


enum class ConnectionState : unsigned short int {
    LOGIN = 0,
    STATUS = 10,
    PLAY = 100,
    HANDSHAKING = 20, DISCONNECT = 999
};

enum class CONNECTION_CONTEXT_TYPE : unsigned short int {
    ACCEPT,
    RECEIVE,
    SEND
};


struct CONNECTION_INFO {
    SOCKET playerSocket;
    ConnectionState connectionState;
};

struct alignas(64) PLAYER_CONNECTION_CONTEXT {
    // Vector for IO buffer, allocated from a memory resource

    // Non-copyable
    PLAYER_CONNECTION_CONTEXT(const PLAYER_CONNECTION_CONTEXT&) = delete;
    PLAYER_CONNECTION_CONTEXT& operator=(const PLAYER_CONNECTION_CONTEXT&) = delete;

    // Constructor with polymorphic allocator
    explicit PLAYER_CONNECTION_CONTEXT(std::pair<char*, size_t> buffer) {
        this->buffer.buf = buffer.first;
        this->buffer.len = buffer.second;
    }


    // Connection info
    CONNECTION_INFO connectionInfo {INVALID_SOCKET, ConnectionState::HANDSHAKING};
    CONNECTION_CONTEXT_TYPE type = CONNECTION_CONTEXT_TYPE::ACCEPT;

    // Overlapped IO
    OVERLAPPED overlapped {};

    // WSABUF to use with Windows socket APIs
    WSABUF buffer{};

    // Reset values as CONTEXT object may be used for other sockets.
    void reset() {
        connectionInfo.playerSocket = INVALID_SOCKET;
        connectionInfo.connectionState = ConnectionState::HANDSHAKING;
        type = CONNECTION_CONTEXT_TYPE::ACCEPT;

        ZeroMemory(buffer.buf, buffer.len);
    }

    void copy(PLAYER_CONNECTION_CONTEXT* context) {
        connectionInfo.playerSocket = context->connectionInfo.playerSocket;
        connectionInfo.connectionState = context->connectionInfo.connectionState;
    }
};



PLAYER_CONNECTION_CONTEXT* borrowContext();

bool sendDataToConnection(WritePacketBuffer* buffer, PLAYER_CONNECTION_CONTEXT* playerSocket);

[[maybe_unused]] bool startNetworkManager(int maxPlayers) noexcept;

[[maybe_unused]] void stopNetworkManager() noexcept;

void closeConnection(PLAYER_CONNECTION_CONTEXT* playerConnectionContext) noexcept;

#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
