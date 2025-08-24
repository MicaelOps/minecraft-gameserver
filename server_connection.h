//
// Created by Micael Cossa on 07/08/2025.
//

#ifndef MINECRAFTSERVER_SERVER_CONNECTION_H
#define MINECRAFTSERVER_SERVER_CONNECTION_H


#include <winsock2.h>
#include "concurrent_unordered_set.h"
#include "packet_buf.h"

#pragma comment(lib, "Ws2_32.lib")



bool sendDataToConnection(PacketBuffer* buffer, SOCKET playerSocket);

bool startupServerNetwork();

void closeServerSocket();

void closeConnection(SOCKET playerSocket);

#endif //MINECRAFTSERVER_SERVER_CONNECTION_H
