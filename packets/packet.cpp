//
// Created by Micael Cossa on 26/07/2025.
//

#include "packet.h"
#include <iostream>

void Packet::readFromBuffer(ReadPacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to readFromBuffer";
}

void Packet::writeToBuffer(WritePacketBuffer* packetBuffer) {
    std::cout << "Unsupported call to writeToBuffer";
}

void Packet::handlePacket(CONNECTION_INFO* connectionInfo){
    std::cout << "Unsupported call to serverHandle";
}

