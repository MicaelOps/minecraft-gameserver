//
// Created by Micael Cossa on 26/07/2025.
//

#include "packets/packet.h"
#include <iostream>

void Packet::readFromBuffer(ReadPacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to readFromBuffer";
}

void Packet::writeToBuffer(WritePacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to writeToBuffer";
}

void Packet::handlePacket(PLAYER_CONNECTION_CONTEXT* connectionContext){
    std::cout << "Unsupported call to serverHandle";
}

