//
// Created by Micael Cossa on 26/07/2025.
//

#include "HandshakePacket.h"
#include "../minecraft.h"
#include "../server_utils.h"
#include "../server_handler.h"

#define PROTOCOL_VERSION_1_8 47

void HandshakePacket::readFromBuffer(ReadPacketBuffer* packetBuffer) {
    protocolVersion = packetBuffer->readVarInt();
}

void HandshakePacket::writeToBuffer(WritePacketBuffer* packetBuffer) {

    response = "{ 'version': { 'name' : '1.8.8','protocol': 47 },'players': {'max': 30,'online': 1,'sample': [  {  'name': 'cakeless',   'id': '0541ed27-7595-4e6a-9101-6c07f879b7b5' } ] }, 'description': { 'text': 'textlul'} ,'favicon': '', 'enforcesSecureChat': false}";
    auto pos = response.find("textlul");
    packetBuffer->writeVarInt(0); // packet id
    packetBuffer->writeString(response.replace(pos, std::string("textlul").length(), Minecraft::getServer().getMOTD())); // temporary until i do the json code);

}

void HandshakePacket::handlePacket(CONNECTION_INFO *connectionInfo) {

    if(protocolVersion != PROTOCOL_VERSION_1_8) {
        printInfo("A non 1.8.x connection has tried to initiate a handshake..");
        closeConnection(connectionInfo->playerSocket);
        return;
    }

    connectionInfo->connectionState = ConnectionState::STATUS;

    sendPacket(this, connectionInfo);

}

