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

    void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) override {
        printf("Call to an undefined Packet! \n");
    }

    void writeToBuffer(WritePacketBuffer* packetBuffer) override {
        printf("Call to an undefined Packet! \n");
    }
    void clear() override;


    UndefinedPacket() = default;
};

void UndefinedPacket::clear() {

}


#endif //CORE_MINESERVER_UNDEFINEDPACKET_H
