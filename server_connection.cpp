
// Created by Micael Cossa on 07/08/2025.
//

#include "server_connection.h"
#include <memory>
#include <mswsock.h>
#include <ws2tcpip.h>
#include "server_utils.h"
#include "server_handler.h"

#define SHUTDOWN_KEY 0x203

#pragma comment(lib, "Ws2_32.lib")
namespace {
    LPFN_ACCEPTEX lpfnAcceptEx = nullptr;
    concurrent_unordered_set<SOCKET> connections;
    HANDLE listenPort = nullptr;
}


enum class CONNECTION_CONTEXT_TYPE : unsigned int {
    ACCEPT , RECEIVE , SEND
};
struct IO_SERVER_CONNECTION_THREAD_PARAM {
    SOCKET listenSocket;
};


struct PLAYER_CONNECTION_CONTEXT {

    explicit PLAYER_CONNECTION_CONTEXT() = default;

    PLAYER_CONNECTION_CONTEXT(const PLAYER_CONNECTION_CONTEXT&) = delete;
    PLAYER_CONNECTION_CONTEXT& operator=(const PLAYER_CONNECTION_CONTEXT&) = delete;

    ~PLAYER_CONNECTION_CONTEXT() {
        delete[] buffer.buf; // context takes ownership of the buffer
    }

    CONNECTION_INFO connectionInfo {INVALID_SOCKET, ConnectionState::HANDSHAKING};

    CONNECTION_CONTEXT_TYPE type = CONNECTION_CONTEXT_TYPE::ACCEPT;
    OVERLAPPED overlapped {};
    WSABUF buffer{};
};

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
void proccessPacket(PLAYER_CONNECTION_CONTEXT* context, DWORD bytesRead) {

    std::unique_ptr<ReadPacketBuffer> packetBuffer = std::make_unique<ReadPacketBuffer>(context->buffer.buf, bytesRead);

    int packetSize = packetBuffer->readVarInt();


    if(bytesRead < packetSize) {
        printInfo("Need to perform more read to full ready this packet. Packet size: ",  packetSize , " Bytes Read: " , bytesRead);
        return;
    }

    printInfo("Received a packet of ", packetSize , " varIntSize and ", bytesRead,  "  bytes were read");

    invokePacket(packetBuffer.get(), &context->connectionInfo);

}

bool sendDataToConnection(WritePacketBuffer* buffer, const CONNECTION_INFO* connectionInfo) {

    // Setup pOverlapped
    const auto connection_context = new PLAYER_CONNECTION_CONTEXT;
    SecureZeroMemory(&connection_context->overlapped, sizeof(OVERLAPPED));

    connection_context->connectionInfo.playerSocket = connectionInfo->playerSocket;
    connection_context->connectionInfo.connectionState = connectionInfo->connectionState;

    connection_context->type = CONNECTION_CONTEXT_TYPE::SEND;

    // Transfer ownership of buffer data to context
    char* packetbuffer = buffer->moveBufferToNull();

    connection_context->buffer.buf = packetbuffer;
    connection_context->buffer.len = buffer->getSize();

    int result = WSASend(connection_context->connectionInfo.playerSocket,
                         &connection_context->buffer,
                         1,
                         nullptr,
                         0,
                         &connection_context->overlapped,
                         nullptr);
    if(result == SOCKET_ERROR) {

        int error = WSAGetLastError();

        if(error != WSA_IO_PENDING){
            printInfo("Unable to send data from socket ", WSAGetLastError());
            return false;
        }
    }

    return true;
}
bool readPacket(const PLAYER_CONNECTION_CONTEXT* context) {

    // Setup pOverlapped;
    const auto connection_context = new PLAYER_CONNECTION_CONTEXT;
    SecureZeroMemory(&connection_context->overlapped, sizeof(OVERLAPPED));

    connection_context->connectionInfo.playerSocket = context->connectionInfo.playerSocket;
    connection_context->connectionInfo.connectionState = context->connectionInfo.connectionState;

    connection_context->type = CONNECTION_CONTEXT_TYPE::RECEIVE;

    connection_context->buffer.len = 512;
    connection_context->buffer.buf = new char[512];

    DWORD flags = 0;
    // read the packet length
    int result = WSARecv(connection_context->connectionInfo.playerSocket,
                         &connection_context->buffer,
                         1,
                         nullptr,
                         &flags,
                         &connection_context->overlapped,
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


DWORD WINAPI  SocketCompletionThreadFunction(LPVOID param) {


    if(param == nullptr) {
        printInfo("Thread initialised with no params");
        return -1;
    }

    // Check the queue for completed accepted connections or others
    auto* params = static_cast<IO_SERVER_CONNECTION_THREAD_PARAM*>(param);

    while(true) {
        DWORD NumBytesSent = 0;
        ULONG CompletionKey;
        LPOVERLAPPED Overlapped;



        bool result = GetQueuedCompletionStatus(listenPort,
                                                &NumBytesSent,
                                                reinterpret_cast<PULONG_PTR>(&CompletionKey),
                                                &Overlapped,
                                                INFINITE);

        if(!result) {
            //printInfo("Failed to get Completion Status");
            continue;
        }

        if(Overlapped == nullptr) {

            if(CompletionKey == SHUTDOWN_KEY)
                break;

            printInfo("Something happened to a connection in the port...");
            continue;
        }

        PLAYER_CONNECTION_CONTEXT* connectionContext = CONTAINING_RECORD(Overlapped, PLAYER_CONNECTION_CONTEXT, overlapped);


        if(connectionContext->type == CONNECTION_CONTEXT_TYPE::ACCEPT)  {

            setsockopt(connectionContext->connectionInfo.playerSocket,
                       SOL_SOCKET,
                       SO_UPDATE_ACCEPT_CONTEXT,
                       reinterpret_cast<const char *>(params->listenSocket),
                       sizeof(params->listenSocket));


            connections.insert(connectionContext->connectionInfo.playerSocket);

        } else if(connectionContext->type == CONNECTION_CONTEXT_TYPE::RECEIVE)  {

            if(NumBytesSent > 0)
                proccessPacket(connectionContext, NumBytesSent);

        }

        if(connectionContext->type != CONNECTION_CONTEXT_TYPE::SEND) {
            result = readPacket(connectionContext);

            if(!result) {
                printInfo("Failed to read packet to connection");
            }
        }

        delete connectionContext;

    }
    return 0;
}



bool acceptConnection(const SOCKET listenSocket, HANDLE port) {
    SOCKET AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (AcceptSocket == INVALID_SOCKET) {
        printInfo("Create of ListenSocket socket failed with error: ", WSAGetLastError());
        return false;
    }

    char* lpOutputBuf = new char[2 * (sizeof(SOCKADDR_IN)) + 32]; // Must be valid!
    DWORD bytesReceived = 0;

    // Setup pOverlapped
    const auto connection_context = new PLAYER_CONNECTION_CONTEXT;
    SecureZeroMemory(&connection_context->overlapped, sizeof(OVERLAPPED));

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

    connection_context->type = CONNECTION_CONTEXT_TYPE::ACCEPT;
    connection_context->connectionInfo.playerSocket = AcceptSocket;

    HANDLE acceptPort = CreateIoCompletionPort((HANDLE) AcceptSocket, port, (u_long) 0, 0);

    if (acceptPort == nullptr) {
        printInfo("accept associate failed with error: ",GetLastError());
        delete[] lpOutputBuf;
        delete connection_context;
        return false;
    }

    return true;
}


bool startupServerNetwork() {

    setupPacketFactory();

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

    listenPort = CreateIoCompletionPort((HANDLE) listenSocket, nullptr, 0, 0);

    if(listenPort == nullptr) {
        printInfo("Unable to create association port ", GetLastError());
        return false;
    }


    IO_SERVER_CONNECTION_THREAD_PARAM threadparam = {listenSocket};


    HANDLE hThread = CreateThread(nullptr, 0, SocketCompletionThreadFunction, &threadparam, 0, nullptr);
    CloseHandle(hThread);

    connections.insert(listenSocket);

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
        if(!acceptConnection(listenSocket, listenPort)){
            printInfo("Failed to create accept Connection pool");
            return false;
        }
    }

    return true;
}

void closeServerSocket() {
    connections.for_each([](SOCKET socket) {
        closeConnection(socket);
    });


    PostQueuedCompletionStatus(listenPort, 0, SHUTDOWN_KEY, nullptr);

    printInfo("Waiting to clear network worker...");

    Sleep(3000); // Let all the connections shutdown

    CloseHandle(listenPort);
    WSACleanup();
}


void closeConnection(SOCKET playerSocket) {
    CancelIoEx((HANDLE)playerSocket, nullptr);
    closesocket(playerSocket);
    connections.remove(playerSocket);
}