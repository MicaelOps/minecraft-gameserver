//
// Created by Micael Cossa on 30/08/2025.
//
#include <mswsock.h>
#include <thread>
#include <ws2tcpip.h>
#include <memory_resource>
#include <utility>
#import "server_connection.h"
#import "server_handler.h"
#import "server_utils.h"


struct MEMORY_MANAGER {
    alignas(64) std::array<std::byte, 2 * 1024 * 1024> contextBuffer;
    alignas(64) std::array<std::byte, 4 * 1024 * 1024> ioBuffer;
    alignas(64) std::array<std::byte, 1 * 1024 * 1024> packetBuffer;

    std::pmr::monotonic_buffer_resource contextUpstream{contextBuffer.data(), contextBuffer.size()};
    std::pmr::monotonic_buffer_resource ioUpstream{ioBuffer.data(), ioBuffer.size()};
    std::pmr::monotonic_buffer_resource packetUpstream{packetBuffer.data(), packetBuffer.size()};

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
    std::pmr::unsynchronized_pool_resource packetPool{&packetUpstream};

    std::pmr::vector<PLAYER_CONNECTION_CONTEXT*> freeContexts{&contextUpstream};
    std::pmr::vector<std::pair<char*, size_t>> freeIOBuffers{&ioUpstream};

    void init();
    void preAllocateContexts(size_t count);
    void preAllocateIOBuffers(size_t count);
    void preAllocatePacketBuffers(size_t count);



    std::unique_ptr<PLAYER_CONNECTION_CONTEXT> acquireContext();
    void returnContext(PLAYER_CONNECTION_CONTEXT* connectionContext);

    std::unique_ptr<char[]> acquireBuffer();
    void releaseBuffer();

    void reset();
};

struct NETWORK_MANAGER {

    concurrent_unordered_set<SOCKET> clientConnections;

    std::vector<std::jthread> workers;

    HANDLE listenPort = nullptr;
    SOCKET listenSocket = INVALID_SOCKET;


    void closeConnection(SOCKET socket) noexcept;

    void startWorkerThreads();

    [[maybe_unused]] bool startNetworkManager(int maxPlayers) noexcept;
    [[maybe_unused]] void stopNetworkManager() noexcept;

    bool acceptConnection() const noexcept;
};


std::unique_ptr<PLAYER_CONNECTION_CONTEXT> MEMORY_MANAGER::acquireContext() {
    if (freeContexts.empty()) return nullptr;

    auto* ctx = freeContexts.back();
    freeContexts.pop_back();

    return std::unique_ptr<PLAYER_CONNECTION_CONTEXT>(ctx);
}

void MEMORY_MANAGER::returnContext(PLAYER_CONNECTION_CONTEXT* connectionContext) {
    connectionContext->reset();
    freeContexts.push_back(connectionContext);
}

std::unique_ptr<char[]> MEMORY_MANAGER::acquireBuffer() {
    constexpr size_t size = 512;
    void* raw = ioPool.allocate(size, alignof(char));
    auto* buf = static_cast<char*>(raw);

    // wrap in unique_ptr with custom deleter that returns to pool
    return std::unique_ptr<char[]>(
            buf,
            [this, size](char* p) {
                if (p) {
                    ioPool.deallocate(p, size, alignof(char));
                }
            }
    );
}

void MEMORY_MANAGER::releaseBuffer() {

}

void MEMORY_MANAGER::reset() {
    contextUpstream.release();
    ioUpstream.release();
    packetUpstream.release();
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
        new(ptr) PLAYER_CONNECTION_CONTEXT{&ioPool};
        freeContexts.emplace_back(ptr);
    }
}

void MEMORY_MANAGER::preAllocateIOBuffers(size_t count) {
    freeIOBuffers.reserve(count);

    // Pre-allocate common buffer sizes
    constexpr std::array<size_t, 4> COMMON_SIZES = {512, 1024, 4096, 8192};

    for (size_t size : COMMON_SIZES) {
        for (size_t i = 0; i < count / COMMON_SIZES.size(); ++i) {
            char* buf = static_cast<char*>(ioPool.allocate(size, 64));
            freeIOBuffers.emplace_back(buf, size);
        }
    }
}

void MEMORY_MANAGER::preAllocatePacketBuffers(size_t count) {
    // For Minecraft packets, pre-allocate based on common packet sizes
    std::vector<std::pmr::vector<char>*> packets;
    packets.reserve(count);

    constexpr std::array<size_t, 5> PACKET_SIZES = {
            64,    // Keep-alive, simple packets
            256,   // Chat messages
            512,   // Player movement
            2048,  // Inventory updates
            32768  // Chunk data
    };

    for (size_t size : PACKET_SIZES) {
        for (size_t i = 0; i < count / PACKET_SIZES.size(); ++i) {
            auto* packet = std::pmr::polymorphic_allocator<std::pmr::vector<char>>{&packetPool}
                    .allocate(1);

            new(packet) std::pmr::vector<char>{&packetPool};
            packet->reserve(size); // Pre-reserve capacity

            packets.push_back(packet);
        }
    }
}

void MEMORY_MANAGER::init() {
    preAllocateContexts(50);     // 50 connection contexts
    preAllocateIOBuffers(100);   // 100 IO buffers
    preAllocatePacketBuffers(200); // 200 packet buffers
}


namespace {
    LPFN_ACCEPTEX lpfnAcceptEx = nullptr;

    NETWORK_MANAGER networkManager;

    // Every worker thread will have its own memory manager.
    thread_local MEMORY_MANAGER memory_manager;

    constexpr ULONG SHUTDOWN_KEY  = 0x13;


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
            networkManager.acceptConnection(); // Accept a new connection

            delete[] connectionContext->buffer.buf; // delete lpOutputBuf

            break;
        case CONNECTION_CONTEXT_TYPE::RECEIVE:
            if(numberOfBytesTransferred > 0)
                proccessPacket(connectionContext, numberOfBytesTransferred);

            charBuffers.returnToPool(connectionContext->buffer.buf);
            break;
        case CONNECTION_CONTEXT_TYPE::SEND:
            charBuffers.returnToPool(connectionContext->buffer.buf);
            break;
        default:
            std::unreachable();
    }


    if(connectionContext->type != CONNECTION_CONTEXT_TYPE::SEND) {
        if(!readPacket(connectionContext)) {
            printInfo("Failed to read packet to connection");
        }
    }

    //memory_manager.returnContext(connectionContext);
}

void NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION() {

    constexpr unsigned MEMORY_RESET_OPERATION_TRIGGER = 124;
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
                                                INFINITE,
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
            proccessIOCPCompletion(context, completion.dwNumberOfBytesTransferred);
        }

        if (++operationCount % MEMORY_RESET_OPERATION_TRIGGER == 0) {
            memory_manager.reset();
            operationCount = 0;
        }
    }
}

bool NETWORK_MANAGER::acceptConnection() const noexcept {

    auto AcceptSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (AcceptSocket == INVALID_SOCKET) {
        printInfo("Create of ListenSocket socket failed with error: ", WSAGetLastError());
        return false; // nothing has been allocated for cleanup;
    }

    auto lpOutputBuf = new char[2 * (sizeof(SOCKADDR_IN)) + 32]; // Must be valid!
    DWORD bytesReceived = 0;

    bool success = false;

    // Prepare Context
    std::unique_ptr<PLAYER_CONNECTION_CONTEXT> connection_context = memory_manager.acquireContext();
    SecureZeroMemory(&connection_context->overlapped, sizeof(OVERLAPPED));

    HANDLE acceptPort;

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
            printInfo("AcceptEx failed");
            goto error_handling;
        }
    }

    connection_context->buffer.buf = lpOutputBuf;
    connection_context->type = CONNECTION_CONTEXT_TYPE::ACCEPT;
    connection_context->connectionInfo.playerSocket = AcceptSocket;

    acceptPort = CreateIoCompletionPort((HANDLE) AcceptSocket, listenPort, (u_long) 0, 0);

    if (acceptPort == nullptr) {
        printInfo("accept associate failed with error: ",GetLastError());
        goto error_handling;
    }

    success = true;
    connection_context.release(); // IOCP will handle the deletion.

    error_handling:
        if(!success) {
            closesocket(AcceptSocket);
            delete[] lpOutputBuf;
            memory_manager.returnContext(connection_context.release());
        }

    return success;
}


[[maybe_unused]] bool NETWORK_MANAGER::startNetworkManager(int maxPlayers) noexcept {
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

    this->listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);

    if(this->listenSocket == INVALID_SOCKET) {
        printInfo("Unable to create socket ", WSAGetLastError());
        goto error_handling;
    }

    this->listenPort = CreateIoCompletionPort((HANDLE) listenSocket, nullptr, 0, 0);

    if(this->listenPort == nullptr) {
        printInfo("Unable to create association port ", GetLastError());
        goto error_handling;
    }

    networkManager.startWorkerThreads();

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

    if (WSAIoctl_RESULT == SOCKET_ERROR) {
        printInfo(" WSAIoctl failed with error: ", WSAGetLastError());
        closesocket(listenSocket);
        goto error_handling;
    }

    // Pre-load a pool of connections
    for (int i = 0; i < maxPlayers; ++i) {
        if(!networkManager.acceptConnection()){
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

[[maybe_unused]] void NETWORK_MANAGER::stopNetworkManager() noexcept {

    // Send a signal to the CompletionThread that it should not process any more connections
    PostQueuedCompletionStatus(this->listenPort, 0, SHUTDOWN_KEY, nullptr);

    // Close every SOCKET Connection
    clientConnections.for_each([this](const SOCKET socket) {
        this->closeConnection(socket);
    });

    printInfo("Waiting to clear network worker...");

    // Wait for all the connections to close
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // Close Completion I/O Port
    CloseHandle(this->listenPort);

    WSACleanup();
}

void NETWORK_MANAGER::closeConnection(const SOCKET socket) noexcept {
    // Cancel any pending I/O Completion requests
    CancelIoEx((HANDLE)socket, nullptr);

    // Terminate connection
    closesocket(socket);

    clientConnections.remove(socket);
}

void NETWORK_MANAGER::startWorkerThreads() {

    unsigned int numThreads = std::thread::hardware_concurrency();
    workers.reserve(numThreads);

    for (int i = 0; i < numThreads; i++) {
        workers.emplace_back(NETWORK_MANAGER_CONNECTION_WORKER_FUNCTION);
    }
}




