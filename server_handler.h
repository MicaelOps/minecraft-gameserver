//
// Created by Micael Cossa on 26/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_HANDLER_H
#define MINECRAFTSERVER_SERVER_HANDLER_H

#include "packets/packet.h"

void setupPacketFactory();
void invokePacket(PacketBuffer* packetBuffer, QUEUED_CONNECTION_CONTEXT* connectionContext);
void sendPacket(Packet* packet, QUEUED_CONNECTION_CONTEXT* connectionContext);

#endif //MINECRAFTSERVER_SERVER_HANDLER_H