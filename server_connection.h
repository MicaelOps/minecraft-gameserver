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
    HANDSHAKING = 20
};

enum class CONNECTION_CONTEXT_TYPE : unsigned short int {
    ACCEPT,
    RECEIVE,
    SEND
};


struct CONNECTION_INFO {
    SOCKET playerSocket;
    ConnectionState connectionState;
    std::unique_ptr<char[]> borrowedBuffer;
};

struct alignas(64) PLAYER_CONNECTION_CONTEXT {
    // Vector for IO buffer, allocated from a memory resource
    std::pmr::vector<char> ioBuffer;

    // Non-copyable
    PLAYER_CONNECTION_CONTEXT(const PLAYER_CONNECTION_CONTEXT&) = delete;
    PLAYER_CONNECTION_CONTEXT& operator=(const PLAYER_CONNECTION_CONTEXT&) = delete;

    // Constructor with polymorphic allocator
    explicit PLAYER_CONNECTION_CONTEXT(std::pmr::memory_resource* mr, size_t reserveSize = 512)
            : ioBuffer(mr)
    {
        ioBuffer.reserve(reserveSize);   // pre-allocate contiguous memory
        buffer.buf = ioBuffer.data();    // point WSABUF to this memory
        buffer.len = static_cast<ULONG>(ioBuffer.capacity());
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

        buffer.buf = nullptr;
        buffer.len = 0;
    }
    // Helper to update WSABUF after resizing vector
    void updateBuffer() {
        buffer.buf = ioBuffer.data();
        buffer.len = static_cast<ULONG>(ioBuffer.size());
    }
};

char* borrowBuffer();

bool sendDataToConnection(WritePacketBuffer* buffer, const CONNECTION_INFO* playerSocket);

bool startupServerNetwork();

void closeServerSocket();

void closeConnection(SOCKET playerSocket);

#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
