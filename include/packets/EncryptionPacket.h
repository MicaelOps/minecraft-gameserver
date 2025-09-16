//
// Created by Micael Cossa on 15/09/2025.
//

#ifndef CORE_MINESERVER_ENCRYPTIONPACKET_H
#define CORE_MINESERVER_ENCRYPTIONPACKET_H


#include "packet.h"

class EncryptionPacket : public Packet {

public:

    void handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) override;

    void writeToBuffer(WritePacketBuffer* packetBuffer) override;

    void clear() override;
};


#endif //CORE_MINESERVER_ENCRYPTIONPACKET_H
