#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX  // Prevents min/max macro conflicts
#endif

#include "include/minecraft.h"
#include "server_connection.h"
#include "packet_handler.h"


#include <algorithm>


namespace {
    NetworkManager* networkManager = nullptr;
    SERVER_INFO* serverInfo = nullptr;
    std::atomic<std::shared_ptr<std::unordered_set<Player>>>* players = nullptr;
    bool init = false;
}


namespace Minecraft {



    bool startupServer() {

        try {

            if(init)
                return true;

            // Initialize server info
            serverInfo = new SERVER_INFO;

            // Initialize players container
            players = new std::atomic<std::shared_ptr<std::unordered_set<Player>>>(
                    std::make_shared<std::unordered_set<Player>>()
            );

            // Initialize network manager with the packet handler
            networkManager = new NetworkManager(invokePacket, setupPacketFactory);
            networkManager->startNetworkManager(serverInfo->maxPlayers);

            printInfo("Minecraft server components initialized successfully");
            init = true;

        } catch (const std::exception& e) {
            printInfo("Failed to initialize server components: ", e.what());
            return false;
        }

        return true;
    }

    void closeServer() {

        if(!init)
            return;

        if(networkManager) {
            networkManager->stopNetworkManager();

            delete networkManager;
            networkManager = nullptr;
        }

        delete serverInfo;
        serverInfo = nullptr;

        delete players;
        players = nullptr;

        init = false;
    }

    NetworkManager& getNetworkManager() {
        if (!networkManager) {
            throw std::runtime_error("NetworkManager not initialized. Call initializeServer() first.");
        }
        return *networkManager;
    }

    SERVER_INFO getServerInformation() {
        if (!serverInfo) {
            throw std::runtime_error("ServerInfo not initialized. Call initializeServer() first.");
        }
        return *serverInfo;
    }

    const std::atomic<std::shared_ptr<std::unordered_set<Player>>>* getPlayers() {
        if (!players) {
            throw std::runtime_error("Players container not initialized. Call initializeServer() first.");
        }
        return players;
    }

    bool playerExistsByName(const std::string_view& username) {

        if(!players)
            return false;

        const auto& players_ref = *players->load();
        return std::any_of(players_ref.begin(), players_ref.end(), [&username](const Player& player)  { return player.getName() == username;});
    }

    bool playerExistsByUUID(const std::string_view& uuid) {

        if(!players)
            return false;

        const auto& players_ref = *players->load();
        return std::any_of(players_ref.begin(), players_ref.end(), [&uuid](const Player& player)  { return player.getName() == uuid;});
    }
}


void SERVER_INFO::setMOTD(const std::string_view& newmotd) noexcept{
    motd = newmotd;
}

std::string SERVER_INFO::getMOTD() const noexcept {
    return motd;
}