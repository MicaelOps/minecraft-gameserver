//
// Created by Micael Cossa on 26/07/2025.
//

#include "packets/HandshakePacket.h"
#include "minecraft.h"
#include "packet_handler.h"

#define PROTOCOL_VERSION_1_8 47


constexpr int STATUS_INTENTION = 1;
constexpr int LOGIN_INTENTION = 2;



void HandshakePacket::writeToBuffer(WritePacketBuffer* packetBuffer) {

    if(state == STATUS_INTENTION) {

        response = "{ 'version': { 'name' : '1.8.8','protocol': 47 },'players': {'max': 30,'online': 1,'sample': [  {  'name': 'cakeless',   'id': '0541ed27-7595-4e6a-9101-6c07f879b7b5' } ] }, 'description': { 'text': 'textlul'} ,'favicon': '', 'enforcesSecureChat': false}";
        auto pos = response.find("textlul");
        packetBuffer->writeVarInt(0); // packet id

        if(protocolVersion == PROTOCOL_VERSION_1_8) {
            packetBuffer->writeString(response.replace(pos, 7, Minecraft::getServerInformation().getMOTD())); // temporary until i do the json code);
        }

    }
}

void HandshakePacket::handlePacket(ReadPacketBuffer* packetBuffer, PLAYER_CONNECTION_CONTEXT* connectionContext) {

    protocolVersion = packetBuffer->readVarInt();
    serverAddress = packetBuffer->readString();
    port = packetBuffer->readShort();
    state = packetBuffer->readVarInt();

    if(protocolVersion != PROTOCOL_VERSION_1_8) {
        response = "{ 'version': { 'name' : '1.8.8','protocol': 99 },'players': {'max': 30,'online': 1,'sample': [  {  'name': 'BADVERSIONLOL',   'id': '0541ed27-7595-4e6a-9101-6c07f879b7b5' } ] }, 'description': { 'text': 'Incompatible Versions'} ,'favicon': '', 'enforcesSecureChat': false}";
        sendPacket(this, connectionContext);
        Minecraft::getNetworkManager().closeConnection(connectionContext);
        return;
    }

    if(state == STATUS_INTENTION) {
        connectionContext->connectionInfo.connectionState = ConnectionState::STATUS;
        sendPacket(this, connectionContext);
    } else if(state == LOGIN_INTENTION) {
        connectionContext->connectionInfo.connectionState = ConnectionState::LOGIN;
    }
}

void HandshakePacket::clear() {

}

