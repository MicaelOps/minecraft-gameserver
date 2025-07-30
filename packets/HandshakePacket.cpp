//
// Created by Micael Cossa on 26/07/2025.
//

#include "HandshakePacket.h"
#include "../server_utils.h"
#include "../server_handler.h"

#define PROTOCOL_VERSION_1_8 47

void HandshakePacket::readFromBuffer(PacketBuffer* packetBuffer) {

    protocolVersion = packetBuffer->readVarInt();

}

void HandshakePacket::writeToBuffer(PacketBuffer* packetBuffer) {

    response = "{ 'version': { 'name' : '1.8.8','protocol': 47 },'players': {'max': 30,'online': 1,'sample': [  {  'name': 'cakeless',   'id': '0541ed27-7595-4e6a-9101-6c07f879b7b5' } ] },'favicon': '', 'enforcesSecureChat': false}";

    packetBuffer->allocateBuffer((int)(response.length() + sizeof(PACKET_ID_SIZE)));
    packetBuffer->writeVarInt(0); // packet id
    packetBuffer->writeString(response);


}

void HandshakePacket::handlePacket(QUEUED_CONNECTION_CONTEXT* connectionContext) {

    printInfo("Protocol version " , protocolVersion);

    if(protocolVersion != PROTOCOL_VERSION_1_8) {
        printInfo("A non 1.8.x connection has tried to initiate a handshake..");
        closeConnection(connectionContext);
        return;
    }

    sendPacket(this, connectionContext);

}

