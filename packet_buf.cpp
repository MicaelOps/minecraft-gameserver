//
// Created by Micael Cossa on 25/07/2025.
//
#include "packet_buf.h"
#include <algorithm>

#define MAXIMUM_VARINT_BITS 5
#define COMPLETION_BIT_MASK 0x80
#define SEGMENT_BITS 0x7F


int ReadPacketBuffer::readVarInt() {
    int final_result = 0, shifts = 0;

    while(position < MAXIMUM_VARINT_BITS && position < size) {

        final_result |= (buffer[position] & SEGMENT_BITS) << (shifts*7);


        position++;

        if ((buffer[position] & COMPLETION_BIT_MASK) != COMPLETION_BIT_MASK) break;

        shifts++;

    }

    return final_result;
}


char *ReadPacketBuffer::getBuffer() {
    return buffer;
}

int ReadPacketBuffer::getSize() const {
    return size;
}

long long int ReadPacketBuffer::readLong() {

    int length = sizeof(long long int);



    if(position + length > size) {
        printf("Unable to readLong, buffer size not big enough position: %d , bufferSize: %d \n", (position + length), size);
        return 0;
    }
    long long value;
    for (int i = 0; i < 8; i++) {
        reinterpret_cast<unsigned char*>(&value)[i] =
                static_cast<unsigned char>(buffer[position + (7 - i)]);
    }

    position+=length;

    return value;
}
// Pre-allocate a buffer of at least 'n' bytes
void WritePacketBuffer::reserve(size_t n) {
    if (capacity < n) {
        char* newBuffer = new char[n];
        if (buffer != nullptr) {
            memcpy(newBuffer, buffer, currPos); // preserve existing data
            delete[] buffer;
        }
        buffer = newBuffer;
        capacity = n;
    }
}

char* WritePacketBuffer::getBuffer() {
    return buffer;
}

size_t WritePacketBuffer::getSize() const {
    return currPos;
}

void WritePacketBuffer::writeByte(const char& byte) {
    if (currPos >= capacity) {
        // grow buffer exponentially if needed
        reserve(capacity > 0 ? capacity * 2 : 512);
    }
    buffer[currPos++] = byte;
}

void WritePacketBuffer::writeVarIntAtTheFront(int value) {
    // Determine the number of bytes needed for the VarInt
    size_t varIntSize = 0;
    int temp = value;
    do {
        temp >>= 7;
        varIntSize++;
    } while (temp != 0);

    // Make sure there’s enough space
    if (currPos + varIntSize > capacity) {
        reserve(currPos + varIntSize);
    }

    // Shift existing data forward to make room for the VarInt
    memmove(buffer + varIntSize, buffer, currPos);

    // Write VarInt bytes at the beginning
    size_t pos = 0;
    do {
        int transfer_bits = value & SEGMENT_BITS;
        value >>= 7;
        if (value != 0)
            transfer_bits |= COMPLETION_BIT_MASK;
        buffer[pos++] = static_cast<char>(transfer_bits);
    } while (value != 0);

    // Update current position
    currPos += varIntSize;
}

void WritePacketBuffer::writeString(const std::string &value) {


    // Write Size
    writeVarInt(static_cast<int>(value.length()));

    // Write UTF-8 Characters
    std::for_each(value.begin(), value.end(), [this](const char& c) {
        this->writeByte(c);
    });
}

void WritePacketBuffer::writeVarInt(int value) {

    do {
        int transfer_bits = (value & SEGMENT_BITS);

        value >>= 7;

        if(value != 0)
            transfer_bits |= COMPLETION_BIT_MASK;

        writeByte(static_cast<char>(transfer_bits));

    } while(value != 0);
}

void WritePacketBuffer::writeLong(const long long int value) {
    int sizeOfLong = sizeof(value);

    char longBuffer[sizeOfLong];

    memcpy(longBuffer, &value, sizeOfLong);

    for(auto byte : longBuffer) {
        writeByte(byte);
    }
}

char *WritePacketBuffer::moveBufferToNull() {
    char* oldbuffer = buffer;
    buffer = nullptr;
    return oldbuffer;
}

