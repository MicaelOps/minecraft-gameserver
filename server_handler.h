//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_HANDLER_H
#define MINECRAFTSERVER_SERVER_HANDLER_H

#include "packets/packet.h"

void setupPacketFactory();
void invokePacket(PacketBuffer* packetBuffer, SOCKET connectionContext);
void sendPacket(Packet* packet, SOCKET playerSocket);

#endif //MINECRAFTSERVER_SERVER_HANDLER_H