//
// Created by Micael Cossa on 21/07/2025.
//

#ifndef MINECRAFTSERVER_PACKET_BUF_H
#define MINECRAFTSERVER_PACKET_BUF_H


#include <string>
#include <vector>


class ReadPacketBuffer{

private:
    char* buffer;
    int size, position = 0;

public:

    explicit ReadPacketBuffer(char* buffer, int size) : buffer(buffer) , size(size) {}

    char* getBuffer();
    int getSize() const;
    int readVarInt();
    long long int readLong();
};

/**
 * The PLAYER_CONNECTION_CONTEXT is responsible for deleting the buffer.
 */
class WritePacketBuffer {

    char* buffer = new char[1];
    size_t capacity = 0;
    size_t currPos = 0;

public:


    char* getBuffer();
    size_t getSize() const;

    char* moveBufferToNull(); // transfer the ownership of our buffer (to PLAYER_CONNECTION_CONTEXT) so this object can be deleted
    void reserve(size_t n);
    void writeVarIntAtTheFront(int value);
    void writeVarInt(int value);
    void writeString(const std::string& value);
    void writeByte(const char &byte);

    void writeLong(long long int i);
};



#endif //MINECRAFTSERVER_PACKET_BUF_H
