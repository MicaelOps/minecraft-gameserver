#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX  // Prevents min/max macro conflicts
#endif

#include "include/minecraft.h"
#include "minecraft_internal.h"
#include "packet_handler.h"

#include <algorithm>


namespace {
    std::unique_ptr<NetworkManager> networkManager = nullptr;
    std::unique_ptr<SERVER_INFO> serverInfo = nullptr;
    std::atomic<std::shared_ptr<std::unordered_set<Player>>> players;
    bool init = false;


}


namespace Minecraft {



    bool startupServer() {

        try {

            if(init)
                return true;

            // Initialize server info
            serverInfo = std::make_unique<SERVER_INFO>();


            players.store(std::make_shared<std::unordered_set<Player>>());

            // Initialize network manager with the packet handler
            networkManager = std::make_unique<NetworkManager>(invokePacket, setupPacketFactory);
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
            networkManager.reset();
        }

        serverInfo.reset();

        init = false;
    }

    SERVER_INFO getServerInformation() {
        if (!serverInfo) {
            throw std::runtime_error("ServerInfo not initialized. Call initializeServer() first.");
        }
        return *serverInfo;
    }

    const std::atomic<std::shared_ptr<std::unordered_set<Player>>>& getPlayers() {
        return players;
    }

    bool playerExistsByName(const std::string_view& username) {

        auto players_ref = players.load();

        if(!players_ref)
            return false;

        return std::any_of(players_ref->begin(), players_ref->end(), [&username](const Player& player)  { return player.getName() == username;});
    }

    bool playerExistsByUUID(const std::string_view& uuid) {

        auto players_ref = players.load();

        if(!players_ref)
            return false;

        return std::any_of(players_ref->begin(), players_ref->end(), [&uuid](const Player& player)  { return player.getUUID() == uuid;});
    }
}

NetworkManager& getNetworkManager() {
    if (!networkManager) {
        throw std::runtime_error("NetworkManager not initialized. Call initializeServer() first.");
    }
    return *networkManager;
}

void SERVER_INFO::setMOTD(const std::string_view& newmotd) noexcept{
    motd = newmotd;
}

std::string SERVER_INFO::getMOTD() const noexcept {
    return motd;
}