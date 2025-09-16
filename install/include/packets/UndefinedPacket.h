//
// Created by Micael Cossa on 27/08/2025.
//

#ifndef CORE_MINESERVER_UNDEFINEDPACKET_H
#define CORE_MINESERVER_UNDEFINEDPACKET_H

#include "packet.h"

/**
 * This packet is to prevent undefined behaviour from arrays garbage values.
 *
 */
class UndefinedPacket : public Packet {

public:
    void readFromBuffer(ReadPacketBuffer* packetBuffer) override  {
        printf("Call to an undefined Packet! \n");
    }

    void handlePacket(PLAYER_CONNECTION_CONTEXT* connectionContext) override {
        printf("Call to an undefined Packet! \n");
    }

    void writeToBuffer(WritePacketBuffer* packetBuffer) override {
        printf("Call to an undefined Packet! \n");
    }


    UndefinedPacket() = default;
};


#endif //CORE_MINESERVER_UNDEFINEDPACKET_H
