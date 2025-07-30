//
// Created by Micael Cossa on 25/07/2025.
//
#include "packet_buf.h"

#define MAXIMUM_VARINT_BITS 5
#define COMPLETION_BIT_MASK 0x80
#define SEGMENT_BITS 0x7F


int PacketBuffer::writeVarInt(int value) {

    do {
        int transfer_bits = (value & (SEGMENT_BITS));
        value >>= 7;

        // insert completion bit
        if(value != 0) {
            transfer_bits |= COMPLETION_BIT_MASK;
        }

        buffer[position] = static_cast<char>(transfer_bits);
        bytesUsed+=8;
        position++;
    } while(value != 0);
    return position;
}

int PacketBuffer::readVarInt() {
    int final_result = 0, shifts = 0;

    while(position <= MAXIMUM_VARINT_BITS) {

        final_result |= (buffer[position] & SEGMENT_BITS) << (shifts*7);

        position++;

        if ((buffer[position] & COMPLETION_BIT_MASK) == 0) break;

        shifts++;

    }

    return final_result;
}

int PacketBuffer::writeShort(unsigned short value) {
    return 0;
}

int PacketBuffer::writeString(std::string &value) {
    writeVarInt(static_cast<int>(value.length()));

    return 0;
}

void PacketBuffer::allocateBuffer(int estimatedSize) {
    buffer = new char[estimatedSize];
}

void PacketBuffer::readjustBufferSize() {

}
