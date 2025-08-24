//
// Created by Micael Cossa on 25/07/2025.
//
#include "packet_buf.h"
#include <algorithm>

#define MAXIMUM_VARINT_BITS 5
#define COMPLETION_BIT_MASK 0x80
#define SEGMENT_BITS 0x7F


int ArrayPacketBuffer::readVarInt() {
    int final_result = 0, shifts = 0;

    while(position < MAXIMUM_VARINT_BITS) {

        final_result |= (buffer[position] & SEGMENT_BITS) << (shifts*7);


        position++;

        if ((buffer[position] & COMPLETION_BIT_MASK) != COMPLETION_BIT_MASK) break;

        shifts++;

    }

    return final_result;
}


char *ArrayPacketBuffer::getBuffer() {
    return buffer;
}

int ArrayPacketBuffer::getSize() {
    return size;
}


char *VectorBuffer::getBuffer() {
    return buffer.data();
}

int VectorBuffer::getSize() {
    return buffer.size();
}

void VectorBuffer::writeVarIntAttheBack(int value) {
    int copyofvalue = value;
    int shifts = 0;
    do {
        int transfer_bits = (value & SEGMENT_BITS);

        value >>= 7;

        if(value != 0)
            transfer_bits |= COMPLETION_BIT_MASK;

        printf("writeVarIntAttheBack called with value %d, byte written to vector %d \n", copyofvalue, transfer_bits);
        buffer.insert(buffer.begin()+shifts, static_cast<char>(transfer_bits));

        shifts++;
    } while(value != 0);
}

void VectorBuffer::writeByte(const char &byte) {
    buffer.push_back(byte);
}

void VectorBuffer::writeString(const std::string &value) {

    printf("writeString length string being written.. \n");
    // Write Size
    writeVarInt(static_cast<int>(value.length()));
    printf("writeString length finished \n");
    // Write UTF-8 Characters
    std::for_each(value.begin(), value.end(), [this](const char& c) {
        this->writeByte(c);
        printf("writeString character byte  %d written \n", c);
    });
}

void VectorBuffer::writeVarInt(int value) {
    int copyofvalue = value;
    do {
        int transfer_bits = (value & SEGMENT_BITS);

        value >>= 7;

        if(value != 0)
            transfer_bits |= COMPLETION_BIT_MASK;

        printf("writeVARInt called with value %d, byte written to vector %d \n", copyofvalue, transfer_bits);
        writeByte(static_cast<char>(transfer_bits));

    } while(value != 0);
}

void PacketBuffer::writeString(const std::string &value) {
    printf("Unsupported call to PacketBuffer writeString");
}

void PacketBuffer::writeByte(const char &byte) {
    printf("Unsupported call to PacketBuffer writeByte");
}

void PacketBuffer::writeShort(const unsigned short& value) {
    printf("Unsupported call to PacketBuffer writeShort");
}

void PacketBuffer::writeVarInt(int value) {
    printf("Unsupported call to PacketBufferwriteVarInt ");
}

void PacketBuffer::writeVarIntAttheBack(int value) {
    printf("Unsupported call to PacketBuffer writeVarIntAttheBack");
}

int PacketBuffer::getSize() {
    printf("Unsupported call to PacketBuffer getSize");
    return 0;
}

char *PacketBuffer::getBuffer() {
    printf("Unsupported call to PacketBuffer getBuffer ");
    return nullptr;
}

int PacketBuffer::readVarInt() {
    printf("Unsupported call to PacketBuffer readVarInt");
    return 0;
}
