//
// Created by Micael Cossa on 26/07/2025.
//

#include "packets/packet.h"
#include <iostream>


void Packet::writeToBuffer(WritePacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to writeToBuffer";
}

void Packet::handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext){
    std::cout << "Unsupported call to serverHandle";
}

void Packet::clear() {
    std::cout << "Unsupported call to clear";
}

