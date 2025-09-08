//
// Created by Micael Cossa on 26/07/2025.
//

#include "packets/HandshakePacket.h"
#include "minecraft.h"
#include "packet_handler.h"

#define PROTOCOL_VERSION_1_8 47


constexpr int STATUS_INTENTION = 1;
constexpr int LOGIN_INTENTION = 2;

void HandshakePacket::readFromBuffer(ReadPacketBuffer* packetBuffer) {
    protocolVersion = packetBuffer->readVarInt();
    serverAddress = packetBuffer->readString();
    port = packetBuffer->readShort();
    printInfo("reading state is next");
    state = packetBuffer->readVarInt();
    printInfo("Handshake Packet ");
    printInfo("Protocol Version:", protocolVersion);
    printInfo("Host:", serverAddress);
    printInfo("port:", port);
    printInfo("State intention:", state);

}

void HandshakePacket::writeToBuffer(WritePacketBuffer* packetBuffer) {

    if(state == STATUS_INTENTION) {
        response = "{ 'version': { 'name' : '1.8.8','protocol': 47 },'players': {'max': 30,'online': 1,'sample': [  {  'name': 'cakeless',   'id': '0541ed27-7595-4e6a-9101-6c07f879b7b5' } ] }, 'description': { 'text': 'textlul'} ,'favicon': '', 'enforcesSecureChat': false}";
        auto pos = response.find("textlul");
        packetBuffer->writeVarInt(0); // packet id
        packetBuffer->writeString(response.replace(pos, std::string("textlul").length(), Minecraft::getServer().getMOTD())); // temporary until i do the json code);

    }
}

void HandshakePacket::handlePacket(PLAYER_CONNECTION_CONTEXT* connectionContext) {

    if(protocolVersion != PROTOCOL_VERSION_1_8) {
        printInfo("A non 1.8.x connection has tried to initiate a handshake..");
        closeConnection(connectionContext);
        return;
    }

    if(state == STATUS_INTENTION) {
        connectionContext->connectionInfo.connectionState = ConnectionState::STATUS;
        sendPacket(this, connectionContext);
    } else if(state == LOGIN_INTENTION) {
        //connectionContext->connectionInfo.connectionState = ConnectionState::LOGIN;
        closeConnection(connectionContext);
    }
}

