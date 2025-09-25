//
// Created by Micael Cossa on 30/08/2025.
//

#include "server_connection.h"
#include "server_utils.h"
#include <thread>

#include <memory_resource>
#include <utility>
#include <chrono>


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
    void returnContext(HANDLE listenport, PLAYER_CONNECTION_CONTEXT* connectionContext);

    std::pair<char *, size_t> acquireBuffer();
    void releaseBuffer(std::pair<char*, size_t> iobuffer);

    void reset();
};


namespace {
    LPFN_ACCEPTEX lpfnAcceptEx = nullptr;

    thread_local MEMORY_MANAGER memory_manager;

    constexpr ULONG SHUTDOWN_KEY  = 0x13;
    constexpr ULONG RETURN_CONTEXT_KEY = 0x14;

}




PLAYER_CONNECTION_CONTEXT* MEMORY_MANAGER::acquireContext() {

    if (freeContexts.empty()) return nullptr;
    auto* ctx = freeContexts.back();
    freeContexts.pop_back();

    return ctx;
}


void MEMORY_MANAGER::returnContext(HANDLE listenport, PLAYER_CONNECTION_CONTEXT* connectionContext) {
    if(connectionContext->contextOwner == std::this_thread::get_id()) {
        connectionContext->reset();
        releaseBuffer(std::make_pair(connectionContext->buffer.buf, connectionContext->buffer.len));
        freeContexts.push_back(connectionContext);
    } else
        PostQueuedCompletionStatus(listenport, 0, RETURN_CONTEXT_KEY, &connectionContext->overlapped);
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
        new(ptr) PLAYER_CONNECTION_CONTEXT{acquireBuffer(), std::this_thread::get_id()};
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



void proccessPacket(NetworkManager* manager, PLAYER_CONNECTION_CONTEXT* context, DWORD bytesRead) {
    ReadPacketBuffer packetBuffer = ReadPacketBuffer(context->buffer.buf, bytesRead);

    int packetSize = packetBuffer.readVarInt();

    if(bytesRead < packetSize) {
        printInfo("Need to perform more read to full ready this packet. Packet size: ",  packetSize , " Bytes Read: " , bytesRead);
        manager->closeConnection(context); //will handle this another time!
        return;
    }

    manager->receiveDataEventHandler(&packetBuffer, context);
}

bool NetworkManager::sendDataToConnection(PLAYER_CONNECTION_CONTEXT* connection_context) {

    connection_context->overlapped = {};
    connection_context->type = CONNECTION_CONTEXT_TYPE::SEND;

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



bool acceptConnection(SOCKET listenSocket, HANDLE listenPort)  noexcept {

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
            listenSocket,
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


    acceptPort = CreateIoCompletionPort((HANDLE) AcceptSocket, listenPort, (u_long) 0, 0);

    if (acceptPort == nullptr) {
        printInfo("accept associate failed with error: ",GetLastError());
        goto error_handling;
    }

    success = true;

    error_handling:
        if(!success) {
            closesocket(AcceptSocket);
            memory_manager.returnContext(listenPort, connection_context);
        }

    return success;
}

void proccessIOCPCompletion(NetworkManager* networkManager, PLAYER_CONNECTION_CONTEXT* connectionContext, DWORD numberOfBytesTransferred) {

    int flag = 1;
    switch(connectionContext->type ) {

        case CONNECTION_CONTEXT_TYPE::ACCEPT:

            setsockopt(connectionContext->connectionInfo.playerSocket,
                       SOL_SOCKET,
                       SO_UPDATE_ACCEPT_CONTEXT,
                       reinterpret_cast<const char *>(networkManager->listenSocket),
                       sizeof(networkManager->listenSocket));


            setsockopt(connectionContext->connectionInfo.playerSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

            networkManager->clientConnections.insert(connectionContext->connectionInfo.playerSocket);
            readPacket(connectionContext);
            acceptConnection(networkManager->listenSocket, networkManager->listenPort); // Accept a new connection
            break;

        case CONNECTION_CONTEXT_TYPE::RECEIVE:


            if(numberOfBytesTransferred == 0 )
                networkManager->closeConnection(connectionContext);

            else {

                proccessPacket(networkManager, connectionContext, numberOfBytesTransferred);

                if(connectionContext->connectionInfo.connectionState != ConnectionState::DISCONNECT)
                    readPacket(connectionContext);
            }


            break;
        case CONNECTION_CONTEXT_TYPE::SEND:

            memory_manager.returnContext(networkManager->listenPort, connectionContext); // send contexts are disposable as they do nothing while read contexts are reusable in the same instance
            break;

        default:
            std::unreachable(); // fancy modern c++ function tsk tsk
    }

}
void NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION(const std::stop_token& token, NetworkManager* networkManager, unsigned int threadId) {

    constexpr unsigned MEMORY_RESET_OPERATION_TRIGGER = 1000;
    unsigned int operationCount = 0;
    memory_manager.init(); // pre-allocate the contiguous pool
    networkManager->loadIocpWorkerThreadEventHandler(); // setup packetfactory for the worker thread

    while(true) {

        constexpr size_t BATCH_SIZE = 16;
        ULONG numOfCompletions = 0;
        std::array<OVERLAPPED_ENTRY, BATCH_SIZE> completionPortsBatch{};

        // Batch processing for better cache locality
        bool result = GetQueuedCompletionStatusEx(networkManager->listenPort,
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
                    printInfo("Network worker", std::this_thread::get_id(), " is shutting down...");
                    return;
                }
                continue;
            }


            auto* context = CONTAINING_RECORD(completion.lpOverlapped, PLAYER_CONNECTION_CONTEXT, overlapped);

            if(completion.lpCompletionKey == RETURN_CONTEXT_KEY) {
                memory_manager.returnContext(networkManager, context);
                continue;
            }

            auto errorCode = static_cast<DWORD>(completion.Internal);

            if (errorCode != ERROR_SUCCESS) {
                printInfo("I/O failed with error: ", errorCode);
                networkManager->closeConnection(context);
                continue;
            }

            proccessIOCPCompletion(networkManager, context, completion.dwNumberOfBytesTransferred);
        }

        if (++operationCount % MEMORY_RESET_OPERATION_TRIGGER == 0) {
            printInfo("Refreshing Network Memory pool...");
            memory_manager.reset();
            operationCount = 0;
        }
    }
}

void NetworkManager::startWorkerThreads() {

    unsigned int numThreads = std::thread::hardware_concurrency()/4;

    workers.reserve(numThreads);

    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back(NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION, this, i);
    }
}
bool NetworkManager::startNetworkManager(int maxPlayers) noexcept {


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

    listenSocket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if(listenSocket == INVALID_SOCKET) {
        printInfo("Unable to create socket ", WSAGetLastError());
        goto error_handling;
    }

    listenPort = CreateIoCompletionPort((HANDLE) listenSocket, nullptr, 0, 0);

    if(listenPort == nullptr) {
        printInfo("Unable to create association port ", GetLastError());
        goto error_handling;
    }

    startWorkerThreads();

    service.sin_family = AF_INET;
    service.sin_port = htons(25565);
    inet_pton(AF_INET, "127.0.0.1", &service.sin_addr);

    if (bind(listenSocket,(SOCKADDR *) & service, sizeof (service)) == SOCKET_ERROR){
        printInfo("bind failed with error: ", WSAGetLastError());
        goto error_handling;
    }


    if (listen( listenSocket, 300 ) == SOCKET_ERROR ) {
        printInfo("LISTEN  failed with error: ", WSAGetLastError());
        goto error_handling;
    }


    WSAIoctl_RESULT = WSAIoctl(listenSocket,
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
        closesocket(listenSocket);
        goto error_handling;
    }

    // Pre-load a pool of connections
    for (int i = 0; i < maxPlayers; ++i) {
        if(!acceptConnection(listenSocket, listenPort)){
            printInfo("Failed to create accept Connection pool");
            goto error_handling;
        }
    }

    success = true;

    error_handling:

        if(!success) {
            closesocket(listenSocket);
            WSACleanup();
        }

    return success;
}


void NetworkManager::closeSocketConnection(SOCKET socket) {
    // Cancel any pending I/O Completion requests
    CancelIoEx((HANDLE) socket, nullptr);

    // Terminate connection
    closesocket(socket);

    clientConnections.remove(socket);
}
void NetworkManager::closeConnection(PLAYER_CONNECTION_CONTEXT* playerConnectionContext) noexcept {

    playerConnectionContext->connectionInfo.connectionState = ConnectionState::DISCONNECT;

    closeSocketConnection(playerConnectionContext->connectionInfo.playerSocket);

    memory_manager.returnContext(this, playerConnectionContext);
}

void NetworkManager::stopNetworkManager() noexcept {

    printInfo("Waiting to clear network workers...");

    for (auto& worker : workers) {
        // Send a signal to the CompletionThread that it should not process any more connections
        PostQueuedCompletionStatus(listenPort, 0, SHUTDOWN_KEY, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Close every SOCKET Connection
    clientConnections.for_each([this](const SOCKET socket) {
        closeSocketConnection(socket);
    });

    // Close Completion I/O Port
    CloseHandle(listenPort);

    workers.clear();

    WSACleanup();
}


PLAYER_CONNECTION_CONTEXT* NetworkManager::acquireContext() {
    return memory_manager.acquireContext();
}
