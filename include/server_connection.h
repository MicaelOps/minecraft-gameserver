//
// Created by Micael Cossa on 07/08/2025.
//

#ifndef MINECRAFTSERVER_SERVER_CONNECTION_H
#define MINECRAFTSERVER_SERVER_CONNECTION_H

// Linux implementation will come later, maybe when I have a spare laptop or this project is near complete.
#include "networking_imports.h"
#include <memory>
#include "concurrent_unordered_set.h"
#include "packet_buf.h"

enum class ConnectionState : unsigned short int {
    HANDSHAKING = 0,
    STATUS = 1,
    LOGIN = 3,
    PLAY = 20, DISCONNECT = 999

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


    // Non-copyable
    PLAYER_CONNECTION_CONTEXT(const PLAYER_CONNECTION_CONTEXT&) = delete;
    PLAYER_CONNECTION_CONTEXT& operator=(const PLAYER_CONNECTION_CONTEXT&) = delete;

    explicit PLAYER_CONNECTION_CONTEXT(std::pair<char*, size_t> buffer, std::thread::id owner) {
        this->buffer.buf = buffer.first;
        this->buffer.len = buffer.second;
        this->contextOwner = owner;
    }

    // IOCP worker thread responsible for construction and destruction of this struct;
    std::thread::id contextOwner;

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
        type = context->type;
    }
};

using RECEIVE_DATA_EVENT_HANDLER = void (*)(ReadPacketBuffer*, PLAYER_CONNECTION_CONTEXT*);
using LOAD_IOCP_WORKER_THREAD_EVENT_HANDLER = void (*)();

class NetworkManager {

private:

    std::vector<std::jthread> workers;

    void startWorkerThreads();


public:

    explicit NetworkManager(RECEIVE_DATA_EVENT_HANDLER receiveDataHandler, LOAD_IOCP_WORKER_THREAD_EVENT_HANDLER loadIocpWorkerThreadEventHandler) : receiveDataEventHandler(receiveDataHandler), loadIocpWorkerThreadEventHandler(loadIocpWorkerThreadEventHandler){};

    NetworkManager(const NetworkManager& copyin) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

    PLAYER_CONNECTION_CONTEXT*  acquireContext();
    bool sendDataToConnection(PLAYER_CONNECTION_CONTEXT* playerSocket);
    bool startNetworkManager(int maxPlayers) noexcept;
    void stopNetworkManager() noexcept;
    void closeConnection(PLAYER_CONNECTION_CONTEXT* playerConnectionContext) noexcept;
    void closeSocketConnection(SOCKET socket);

    HANDLE listenPort = nullptr;
    SOCKET listenSocket = INVALID_SOCKET;
    concurrent_unordered_set<SOCKET> clientConnections;
    RECEIVE_DATA_EVENT_HANDLER receiveDataEventHandler;
    LOAD_IOCP_WORKER_THREAD_EVENT_HANDLER loadIocpWorkerThreadEventHandler;
};




#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
