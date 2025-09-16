//
// Created by Micael Cossa on 30/08/2025.
//
#include "server_connection.h"
#include <mswsock.h>
#include <thread>
#include <ws2tcpip.h>
#include <memory_resource>
#include <utility>
#include <functional>
#include <chrono>
#include "packet_handler.h"
#include "server_utils.h"


struct MEMORY_MANAGER {

    // Contiguous pool of PLAYER_CONNECTION_CONTEXT and char* buffers
    std::array<std::byte, 2 * 1024 * 1024> contextBuffer{};
    std::array<std::byte, 4 * 1024 * 1024> ioBuffer{};


    std::pmr::monotonic_buffer_resource contextUpstream{contextBuffer.data(), contextBuffer.size()};
    std::pmr::monotonic_buffer_resource ioUpstream{ioBuffer.data(), ioBuffer.size()};


    // Pool configurations for pre-allocation
    std::pmr::pool_options contextOptions{
            .max_blocks_per_chunk = 64,  // Pre-allocate 64 contexts at once
            .largest_required_pool_block = sizeof(PLAYER_CONNECTION_CONTEXT)
    };

    std::pmr::pool_options ioOptions{
            .max_blocks_per_chunk = 128, // Pre-allocate 128 IO buffers
            .largest_required_pool_block = 8192 // 8KB buffers
    };

    std::pmr::unsynchronized_pool_resource contextPool{contextOptions, &contextUpstream};
    std::pmr::unsynchronized_pool_resource ioPool{ioOptions, &ioUpstream};

    std::pmr::vector<PLAYER_CONNECTION_CONTEXT*> freeContexts{&contextUpstream};
    std::pmr::vector<std::pair<char*, size_t>> freeIOBuffers{&ioUpstream};

    void init();
    void preAllocateContexts(size_t count);
    void preAllocateIOBuffers(size_t count);




    PLAYER_CONNECTION_CONTEXT* acquireContext();
    void returnContext(PLAYER_CONNECTION_CONTEXT* connectionContext);

    std::pair<char *, size_t> acquireBuffer();
    void releaseBuffer(std::pair<char*, size_t> iobuffer);

    void reset();
};

struct NETWORK_MANAGER {

    concurrent_unordered_set<SOCKET> clientConnections;

    std::vector<std::jthread> workers;

    HANDLE listenPort = nullptr;
    SOCKET listenSocket = INVALID_SOCKET;

};


PLAYER_CONNECTION_CONTEXT* MEMORY_MANAGER::acquireContext() {

    if (freeContexts.empty()) return nullptr;
    auto* ctx = freeContexts.back();
    freeContexts.pop_back();

    return ctx;
}


void MEMORY_MANAGER::returnContext(PLAYER_CONNECTION_CONTEXT* connectionContext) {
    connectionContext->reset();
    releaseBuffer(std::make_pair(connectionContext->buffer.buf, connectionContext->buffer.len));
    freeContexts.push_back(connectionContext);
}

std::pair<char*, size_t> MEMORY_MANAGER::acquireBuffer() {

    if (freeIOBuffers.empty()) return {nullptr, 0};

    auto ctx = freeIOBuffers.back();
    freeIOBuffers.pop_back();

    return ctx;
}

void MEMORY_MANAGER::releaseBuffer(std::pair<char*, size_t> iobuffer) {
    freeIOBuffers.push_back(iobuffer);
}

void MEMORY_MANAGER::reset() {
    contextUpstream.release();
    ioUpstream.release();
}


void MEMORY_MANAGER::preAllocateContexts(size_t count) {
    freeContexts.reserve(count);

    // Allocate contexts to warm up the pool
    for (size_t i = 0; i < count; ++i) {
        auto* ptr = static_cast<PLAYER_CONNECTION_CONTEXT*>(
                contextPool.allocate(sizeof(PLAYER_CONNECTION_CONTEXT),
                                     alignof(PLAYER_CONNECTION_CONTEXT))
        );

        // Construct the object to ensure vtables/etc are set up
        new(ptr) PLAYER_CONNECTION_CONTEXT{acquireBuffer()};
        freeContexts.emplace_back(ptr);
    }
}

void MEMORY_MANAGER::preAllocateIOBuffers(size_t count) {


    // Pre-allocate common buffer sizes
    // This will become useful later on after I do some profiling on the average packet size after player has logged in
    // Especially when sending/receiving chunk data
    constexpr std::array<size_t, 4> COMMON_SIZES = {512, 1024, 4096, 8192};

    freeIOBuffers.reserve(count*COMMON_SIZES.size());

    for (size_t size : COMMON_SIZES) {
        for (size_t i = 0; i < count / COMMON_SIZES.size(); ++i) {
            char* buf = static_cast<char*>(ioPool.allocate(size, 64));
            freeIOBuffers.emplace_back(buf, size);
        }
    }
}

void MEMORY_MANAGER::init() {
    preAllocateIOBuffers(100);   // 100 IO buffers
    preAllocateContexts(50);     // 50 connection contexts
}


namespace {
    LPFN_ACCEPTEX lpfnAcceptEx = nullptr;

    NETWORK_MANAGER networkManager;

    thread_local MEMORY_MANAGER memory_manager;

    constexpr ULONG SHUTDOWN_KEY  = 0x13;
    constexpr ULONG RETURN_CONTEXT_KEY = 0x14;

}

PLAYER_CONNECTION_CONTEXT* borrowContext() {
    return memory_manager.acquireContext();
}

void proccessPacket(PLAYER_CONNECTION_CONTEXT* context, DWORD bytesRead) {
    std::unique_ptr<ReadPacketBuffer> packetBuffer = std::make_unique<ReadPacketBuffer>(context->buffer.buf, bytesRead);

    int packetSize = packetBuffer->readVarInt();

    if(bytesRead < packetSize) {
        printInfo("Need to perform more read to full ready this packet. Packet size: ",  packetSize , " Bytes Read: " , bytesRead);
        closeConnection(context); //will handle this another time!
        return;
    }

    invokePacket(packetBuffer.get(), context);
}

bool sendDataToConnection(PLAYER_CONNECTION_CONTEXT* connection_context) {

    connection_context->overlapped = {};

    int result = WSASend(connection_context->connectionInfo.playerSocket,
                         &connection_context->buffer,
                         1,
                         nullptr,
                         0,
                         &connection_context->overlapped,
                         nullptr);

    if(result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        printInfo("Unable to send data from socket ", WSAGetLastError());
        return false;
    }

    return true;
}

void readPacket(PLAYER_CONNECTION_CONTEXT* context) {

    context->overlapped = {};
    memset(context->buffer.buf, 0, context->buffer.len); // reset buffer


    context->type = CONNECTION_CONTEXT_TYPE::RECEIVE;

    DWORD flags = 0;
    // read the packet length
    int result = WSARecv(context->connectionInfo.playerSocket,
                         &context->buffer,
                         1,
                         nullptr,
                         &flags,
                         &context->overlapped,
                         nullptr);

    if(result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        printInfo("Unable to read data from socket ", WSAGetLastError());

    }
}



bool acceptConnection()  noexcept {

    auto AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (AcceptSocket == INVALID_SOCKET) {
        printInfo("Create of ListenSocket socket failed with error: ", WSAGetLastError());
        return false; // nothing has been allocated for cleanup;
    }

    DWORD bytesReceived = 0;

    bool success = false;

    // Prepare Context
    auto* connection_context = memory_manager.acquireContext();

    if (!connection_context) {
        printInfo("Failed to acquire context for sending");
        return false;
    }

    connection_context->overlapped = {};

    HANDLE acceptPort;

    auto result = lpfnAcceptEx(
            networkManager.listenSocket,
            AcceptSocket,
            connection_context->buffer.buf,
            0,
            sizeof(SOCKADDR_IN) + 16,
            sizeof(SOCKADDR_IN) + 16,
            &bytesReceived,
            &connection_context->overlapped
    );


    if(!result) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
            printInfo("AcceptEx failed");
            goto error_handling;
        }
    }

    connection_context->type = CONNECTION_CONTEXT_TYPE::ACCEPT;
    connection_context->connectionInfo.playerSocket = AcceptSocket;
    connection_context->connectionInfo.connectionState = ConnectionState::HANDSHAKING;


    acceptPort = CreateIoCompletionPort((HANDLE) AcceptSocket, networkManager.listenPort, (u_long) 0, 0);

    if (acceptPort == nullptr) {
        printInfo("accept associate failed with error: ",GetLastError());
        goto error_handling;
    }

    success = true;

    error_handling:
        if(!success) {
            closesocket(AcceptSocket);
            memory_manager.returnContext(connection_context);
        }

    return success;
}

void proccessIOCPCompletion(PLAYER_CONNECTION_CONTEXT* connectionContext, DWORD numberOfBytesTransferred) {

    int flag = 1;
    switch(connectionContext->type ) {

        case CONNECTION_CONTEXT_TYPE::ACCEPT:

            setsockopt(connectionContext->connectionInfo.playerSocket,
                       SOL_SOCKET,
                       SO_UPDATE_ACCEPT_CONTEXT,
                       reinterpret_cast<const char *>(networkManager.listenSocket),
                       sizeof(networkManager.listenSocket));


            setsockopt(connectionContext->connectionInfo.playerSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

            networkManager.clientConnections.insert(connectionContext->connectionInfo.playerSocket);
            readPacket(connectionContext);
            acceptConnection(); // Accept a new connection
            break;

        case CONNECTION_CONTEXT_TYPE::RECEIVE:


            if(numberOfBytesTransferred == 0 ){
                printInfo("0 bytes were read for a connection, shutting down gracefully");
                closeConnection(connectionContext);
            } else {
                proccessPacket(connectionContext, numberOfBytesTransferred);

                if(connectionContext->connectionInfo.connectionState != ConnectionState::DISCONNECT)
                    readPacket(connectionContext);
            }


            break;
        case CONNECTION_CONTEXT_TYPE::SEND:

            memory_manager.returnContext(connectionContext); // send contexts are disposable as they do nothing while read contexts are reusable in the same instance
            break;

        default:
            std::unreachable(); // fancy modern c++ function tsk tsk
    }

}
void NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION(const std::stop_token& token, unsigned int threadId) {

    constexpr unsigned MEMORY_RESET_OPERATION_TRIGGER = 1000;
    unsigned int operationCount = 0;
    memory_manager.init(); // pre-allocate the contiguous pool


    while(true) {

        constexpr size_t BATCH_SIZE = 16;
        ULONG numOfCompletions = 0;
        std::array<OVERLAPPED_ENTRY, BATCH_SIZE> completionPortsBatch{};

        // Batch processing for better cache locality
        bool result = GetQueuedCompletionStatusEx(networkManager.listenPort,
                                                completionPortsBatch.data(),
                                                BATCH_SIZE,
                                                &numOfCompletions,
                                                1,
                                                FALSE);

        if(!result)
            //printInfo("Failed to get Completion Status");
            continue;


        for(ULONG i = 0; i < numOfCompletions; ++i) {

            auto& completion = completionPortsBatch[i];

            if (completion.lpOverlapped == nullptr) {
                if (completion.lpCompletionKey == SHUTDOWN_KEY) {
                    return;
                }
                continue;
            }


            auto* context = CONTAINING_RECORD(completion.lpOverlapped, PLAYER_CONNECTION_CONTEXT, overlapped);
            auto errorCode = static_cast<DWORD>(completion.Internal);

            if (errorCode != ERROR_SUCCESS) {
                printInfo("I/O failed with error: ", errorCode);
                closeConnection(context);
                continue;
            }

            proccessIOCPCompletion(context, completion.dwNumberOfBytesTransferred);
        }

        if (++operationCount % MEMORY_RESET_OPERATION_TRIGGER == 0) {
            printInfo("reset??");
            memory_manager.reset();
            operationCount = 0;
        }
    }
}

void startWorkerThreads() {

    unsigned int numThreads = std::thread::hardware_concurrency()/4;

    networkManager.workers.reserve(numThreads);

    for (int i = 0; i < numThreads; i++) {
        networkManager.workers.emplace_back(NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION, i);
    }
}
[[maybe_unused]] bool startNetworkManager(int maxPlayers) noexcept {
    setupPacketFactory();


    WSADATA wsadata;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytesReturned;
    int WSAIoctl_RESULT;
    sockaddr_in service{};

    // has the function being executed successfully?
    bool success = false;

    // Check version
    if (WSAStartup(MAKEWORD(2,2), &wsadata) == 1) {
        printInfo("Unable to start WSA ", WSAGetLastError());
        return false;
    }

    networkManager.listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if(networkManager.listenSocket == INVALID_SOCKET) {
        printInfo("Unable to create socket ", WSAGetLastError());
        goto error_handling;
    }

    networkManager.listenPort = CreateIoCompletionPort((HANDLE) networkManager.listenSocket, nullptr, 0, 0);

    if(networkManager.listenPort == nullptr) {
        printInfo("Unable to create association port ", GetLastError());
        goto error_handling;
    }

    startWorkerThreads();

    service.sin_family = AF_INET;
    service.sin_port = htons(25565);
    inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);

    if (bind(networkManager.listenSocket,(SOCKADDR *) & service, sizeof (service)) == SOCKET_ERROR){
        printInfo("bind failed with error: ", WSAGetLastError());
        goto error_handling;
    }


    if (listen( networkManager.listenSocket, 300 ) == SOCKET_ERROR ) {
        printInfo("LISTEN  failed with error: ", WSAGetLastError());
        goto error_handling;
    }


    WSAIoctl_RESULT = WSAIoctl(networkManager.listenSocket,
                               SIO_GET_EXTENSION_FUNCTION_POINTER,
                           &guidAcceptEx,
                           sizeof (guidAcceptEx),
                           &lpfnAcceptEx,
                           sizeof (lpfnAcceptEx),
                           &bytesReturned,
                           nullptr,
                           nullptr);


    memory_manager.init();

    if (WSAIoctl_RESULT == SOCKET_ERROR) {
        printInfo(" WSAIoctl failed with error: ", WSAGetLastError());
        closesocket(networkManager.listenSocket);
        goto error_handling;
    }

    // Pre-load a pool of connections
    for (int i = 0; i < maxPlayers; ++i) {
        if(!acceptConnection()){
            printInfo("Failed to create accept Connection pool");
            goto error_handling;
        }
    }

    success = true;

    error_handling:

        if(!success) {
            closesocket(networkManager.listenSocket);
            WSACleanup();
        }

    return success;
}


void closeSocketConnection(SOCKET socket) {
    // Cancel any pending I/O Completion requests
    CancelIoEx((HANDLE) socket, nullptr);

    // Terminate connection
    closesocket(socket);

    networkManager.clientConnections.remove(socket);
}
void closeConnection(PLAYER_CONNECTION_CONTEXT* playerConnectionContext) noexcept {

    printInfo("Thread " , std::this_thread::get_id(), " closing connection");

    playerConnectionContext->connectionInfo.connectionState = ConnectionState::DISCONNECT;

    closeSocketConnection(playerConnectionContext->connectionInfo.playerSocket);

    memory_manager.returnContext(playerConnectionContext);
}

[[maybe_unused]] void stopNetworkManager() noexcept {

    // Send a signal to the CompletionThread that it should not process any more connections
    PostQueuedCompletionStatus(networkManager.listenPort, 0, SHUTDOWN_KEY, nullptr);

    // Close every SOCKET Connection
    networkManager.clientConnections.for_each([](const SOCKET socket) {
        closeSocketConnection(socket);
    });

    printInfo("Waiting to clear network worker...");

    // Wait for all the connections to close
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Close Completion I/O Port
    CloseHandle(networkManager.listenPort);

    WSACleanup();
}






