//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_HANDLER_H
#define MINECRAFTSERVER_SERVER_HANDLER_H

#include "packets/packet.h"
void setupPacketFactory();
void invokePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext);
void sendPacket(Packet* packet, PLAYER_CONNECTION_CONTEXT* playerSocket);

#endif //MINECRAFTSERVER_SERVER_HANDLER_H