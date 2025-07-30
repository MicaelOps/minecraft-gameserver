cmake_minimum_required(VERSION 3.26)
project(minecraftserver)

set(CMAKE_CXX_STANDARD 20)

add_executable(minecraftserver main.cpp
        player.cpp
        player.h
        server_connection.h
        server_connection.cpp
        concurrent_unordered_set.cpp
        concurrent_unordered_set.h
        "packet_buf.h"
        packet_buf.cpp
        server_handler.cpp
        server_handler.h
        packets/packet.cpp
        packets/packet.h
        packets/HandshakePacket.cpp
        packets/HandshakePacket.h
        server_utils.cpp
        server_utils.h
        json_reader.cpp
        json_reader.h)

target_link_libraries(minecraftserver PRIVATE ws2_32)
