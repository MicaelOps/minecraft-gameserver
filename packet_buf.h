//
// Created by Micael Cossa on 21/07/2025.
//

#ifndef MINECRAFTSERVER_PACKET_BUF_H
#define MINECRAFTSERVER_PACKET_BUF_H


#include <string>

struct PacketBuffer {

    int position = 0, bufferSize = 0, bytesUsed = 0;
    char* buffer = nullptr;


    int writeVarInt(int value);
    int writeShort(unsigned short value);
    int writeString(std::string& value);

    int readVarInt();

    void readjustBufferSize();
    void allocateBuffer(int estimatedSize);
};



#endif //MINECRAFTSERVER_PACKET_BUF_H
