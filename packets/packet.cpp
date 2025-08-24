//
// Created by Micael Cossa on 26/07/2025.
//

#include "packet.h"
#include <iostream>

void Packet::readFromBuffer(PacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to readFromBuffer";
}

void Packet::writeToBuffer(PacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to writeToBuffer";
}

void Packet::handlePacket(SOCKET playerSocket){
    std::cout << "Unsupported call to serverHandle";
}

