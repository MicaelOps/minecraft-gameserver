//
// Created by Micael Cossa on 18/07/2025.
//

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include "server_utils.h"
#include "server_connection.h"
#include "server_handler.h"

#pragma comment(lib, "Ws2_32.lib")

#define SHUTDOWN_KEY 0x203

static LPFN_ACCEPTEX lpfnAcceptEx = nullptr;



/**
 * Packet format:
 *
 * LENGTH (Packet ID + Data)  - VARINT
 * Packet ID
 * DATA
 *
 * Note: The field of packet length is not included in the total packet length
 * @param context
 * @param bytesRead
 */
void proccessPacket(QUEUED_CONNECTION_CONTEXT* context, DWORD bytesRead) {


    PacketBuffer packetBuffer;
    packetBuffer.buffer = context->buffer.buf;
    packetBuffer.bufferSize = (int)context->buffer.len;

    int packetSize = packetBuffer.readVarInt();

    if(bytesRead < packetSize) {
        printInfo("Need to perform more read to full ready this packet. Packet size: ",  packetSize , " Bytes Read: " , bytesRead);
        return;
    }
    invokePacket(&packetBuffer, context);
}

bool sendDataToConnection(PacketBuffer* buffer, QUEUED_CONNECTION_CONTEXT* context) {

    auto* buf = new WSABUF;
    buf->buf = buffer->buffer;
    buf->len = buffer->bufferSize;

    int result = WSASend(context->sAcceptSocket,
                         buf,
                         1,
                         nullptr,
                         MSG_OOB,
                         &context->overlapped,
                         nullptr);

    if(result == SOCKET_ERROR) {

        int error = WSAGetLastError();

        if(error != WSA_IO_PENDING){
            printInfo("Unable to read data from socket ", WSAGetLastError());
            return false;
        }
    }

    return true;
}
bool readPacket(QUEUED_CONNECTION_CONTEXT* context) {

    context->buffer.len = 512;
    context->buffer.buf = new char[512];
    DWORD flags = MSG_PEEK;


    // read the packet length
    int result = WSARecv(context->sAcceptSocket,
                         &context->buffer,
                         1,
                         nullptr,
                         &flags,
                         &context->overlapped,
                         nullptr);

    if(result == SOCKET_ERROR) {

        int error = WSAGetLastError();

        if(error != WSA_IO_PENDING){
            printInfo("Unable to read data from socket ", WSAGetLastError());
            return false;
        }
    }

    return true;
}

DWORD WINAPI acceptingSocketCompletiontFunction(LPVOID lpParam) {

    // Check the queue for completed accepted connections or others
    while(true) {
        DWORD NumBytesSent = 0;
        ULONG CompletionKey;
        LPOVERLAPPED Overlapped;

        bool result = GetQueuedCompletionStatus((HANDLE)lpParam,
                                                &NumBytesSent,
                                                reinterpret_cast<PULONG_PTR>(&CompletionKey),
                                                &Overlapped,
                                                INFINITE);

        if(!result) {
            printInfo("Failed to get Completion Status");
            continue;
        }

        if(Overlapped == nullptr) {

            if(CompletionKey == SHUTDOWN_KEY)
                break;

            printInfo("Something happen to a connection in the port...");
            continue;
        }


        QUEUED_CONNECTION_CONTEXT* connectionContext = CONTAINING_RECORD(Overlapped, QUEUED_CONNECTION_CONTEXT, overlapped);

        if(connectionContext->state == ACCEPT)  {
            setsockopt( connectionContext->sAcceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&connectionContext->sListenSocket, sizeof(connectionContext->sListenSocket) );
            connectionContext->connection->connections.insert(connectionContext->sAcceptSocket);
            connectionContext->state = HANDSHAKING;
            readPacket(connectionContext);
        } else  {
            proccessPacket(connectionContext, NumBytesSent);
        }

    }
    return 0;
}

bool acceptConnection(const SOCKET listenSocket, HANDLE port, SERVER_CONNECTION* connections) {
    SOCKET AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (AcceptSocket == INVALID_SOCKET) {
        printInfo("Create of ListenSocket socket failed with error: ", WSAGetLastError());
        return false;
    }

    char* lpOutputBuf = new char[2 * (sizeof(SOCKADDR_IN)) + 32]; // Must be valid!
    DWORD bytesReceived = 0;

    // Setup pOverlapped
    auto connection_context = new QUEUED_CONNECTION_CONTEXT;
    SecureZeroMemory(connection_context, sizeof(QUEUED_CONNECTION_CONTEXT));


    auto result = lpfnAcceptEx(
            listenSocket,
            AcceptSocket,
            lpOutputBuf,
            0,
            sizeof(SOCKADDR_IN) + 16,
            sizeof(SOCKADDR_IN) + 16,
            &bytesReceived,
            &connection_context->overlapped
    );

    if(!result) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
            std::cerr << "AcceptEx failed immediately: ";
            closesocket(AcceptSocket);
            delete[] lpOutputBuf;
            delete connection_context;
            return false;
        }
    }

    connection_context->connection = connections;
    connection_context->sAcceptSocket = AcceptSocket;
    connection_context->sListenSocket = listenSocket;

    HANDLE acceptPort = CreateIoCompletionPort((HANDLE) AcceptSocket, port, (u_long) 0, 0);

    if (acceptPort == nullptr) {
        printInfo("accept associate failed with error: ",GetLastError());
        delete[] lpOutputBuf;
        delete connection_context;
        return false;
    }

    return true;
}
bool startupServerNetwork(SERVER_CONNECTION* serverConnection) {
    WSADATA wsadata;

    if (WSAStartup(MAKEWORD(2,2), &wsadata) == 1) {
        printInfo("Unable to start WSA ", WSAGetLastError());
        return false;
    }


    auto listenSocket = INVALID_SOCKET;

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if(listenSocket == INVALID_SOCKET) {
        printInfo("Unable to create socket ", WSAGetLastError());
        WSACleanup();
        return false;
    }

    HANDLE listenPort = CreateIoCompletionPort((HANDLE) listenSocket, nullptr, 0, 0);

    if(listenPort == nullptr) {
        printInfo("Unable to create association port ", GetLastError());
        return false;
    }


    HANDLE hThread = CreateThread(nullptr, 0, acceptingSocketCompletiontFunction, listenPort, 0, nullptr);
    CloseHandle(hThread);

    serverConnection->listenport = listenPort;
    serverConnection->connections.insert(listenSocket);

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_port = htons(25565);
    inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);

    if (bind(listenSocket,(SOCKADDR *) & service, sizeof (service)) == SOCKET_ERROR){
        printInfo("bind failed with error: ", WSAGetLastError());
        return false;
    }


    if (listen( listenSocket, 300 ) == SOCKET_ERROR ) {
        printInfo("LISTEN  failed with error: ", WSAGetLastError());
        return false;
    }

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytesReturned;

    int iResult = WSAIoctl(listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
                       &guidAcceptEx, sizeof (guidAcceptEx),
                       &lpfnAcceptEx, sizeof (lpfnAcceptEx),
                       &bytesReturned, nullptr, nullptr);

    if (iResult == SOCKET_ERROR) {
        printInfo(" WSAIoctl failed with error: ", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
    // Pre-load a pool of 20 connections
    for (int i = 0; i < 20; ++i) {
        if(!acceptConnection(listenSocket, listenPort,serverConnection)){
            printInfo("Failed to create accept Connection pool");
            return false;
        }
    }

    return true;
}

void closeConnection(QUEUED_CONNECTION_CONTEXT* connectionContext) {
    connectionContext->connection->connections.remove(connectionContext->sAcceptSocket);
    closesocket(connectionContext->sAcceptSocket);
}

void closeServerConnections(SERVER_CONNECTION* serverConnection) {
    printInfo("Shutting down connections...");
    serverConnection->connections.for_each([](SOCKET sock) {
        closesocket(sock);
    });

    serverConnection->connections.clear();
    PostQueuedCompletionStatus(serverConnection->listenport, 0, SHUTDOWN_KEY, nullptr);
    printInfo("Waiting to clear network worker...");
    Sleep(3000); // Let all the thread shutdown
    CloseHandle(serverConnection->listenport);
    WSACleanup();
}