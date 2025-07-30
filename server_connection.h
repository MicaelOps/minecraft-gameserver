//
// Created by Micael Cossa on 18/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_CONNECTION_H
#define MINECRAFTSERVER_SERVER_CONNECTION_H


#include <winsock2.h>
#include "concurrent_unordered_set.h"
#include "packet_buf.h"

enum CONNECTION_STATE {
    ACCEPT, HANDSHAKING, PLAY
};
struct SERVER_CONNECTION {
    HANDLE listenport;
    concurrent_unordered_set<SOCKET> connections;
};

struct QUEUED_CONNECTION_CONTEXT {
    SERVER_CONNECTION* connection = nullptr;
    CONNECTION_STATE state = ACCEPT;
    WSABUF buffer{};
    WSAOVERLAPPED overlapped{};
    SOCKET sAcceptSocket = INVALID_SOCKET;
    SOCKET sListenSocket = INVALID_SOCKET;
};

DWORD WINAPI acceptingSocketCompletiontFunction(LPVOID lpParam);
bool startupServerNetwork(SERVER_CONNECTION* serverConnection);
void closeServerConnections(SERVER_CONNECTION* serverConnection);
void closeConnection(QUEUED_CONNECTION_CONTEXT* connectionContext);
bool sendDataToConnection(PacketBuffer* buffer, QUEUED_CONNECTION_CONTEXT* context);

#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
