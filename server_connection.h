//
// Created by Micael Cossa on 07/08/2025.
//

#ifndef MINECRAFTSERVER_SERVER_CONNECTION_H
#define MINECRAFTSERVER_SERVER_CONNECTION_H

// Linux implementation will come later, maybe when I have a spare laptop or this project is near complete.
#include <winsock2.h>
#include "concurrent_unordered_set.h"
#include "packet_buf.h"

#pragma comment(lib, "Ws2_32.lib")


enum class ConnectionState : unsigned int {

    LOGIN = 0, STATUS = 10, PLAY = 100, HANDSHAKING = 20
};

struct CONNECTION_INFO {
    SOCKET playerSocket = INVALID_SOCKET;
    ConnectionState connectionState = ConnectionState::HANDSHAKING;
};

bool sendDataToConnection(WritePacketBuffer* buffer, const CONNECTION_INFO* playerSocket);

bool startupServerNetwork();

void closeServerSocket();

void closeConnection(SOCKET playerSocket);

#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
