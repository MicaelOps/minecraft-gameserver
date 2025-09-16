#include "include/minecraft.h"
#include "server_connection.h"


#include <algorithm>


SERVER_INFO Minecraft::ServerInformation;

const std::atomic<std::shared_ptr<std::unordered_set<Player>>> Minecraft::Players {
        std::make_shared<std::unordered_set<Player>>()
};

void SERVER_INFO::setMOTD(const std::string_view& newmotd) noexcept{
    motd = newmotd;
}

std::string SERVER_INFO::getMOTD() const noexcept {
    return motd;
}

namespace Minecraft {

    SERVER_INFO getServerInformation() {
        return Minecraft::ServerInformation;
    }

    bool startupServer() {

        if(!startNetworkManager(ServerInformation.maxPlayers)) {
            printInfo("Unable to load network Manager");
            return false;
        }


        return true;
    }


    void closeServer() {
        stopNetworkManager();
    }
    bool playerExistsByName(const std::string_view& username) {
        const auto& players = *Minecraft::Players.load();
        return std::any_of(players.begin(), players.end(), [&username](const Player& player)  { return player.getName() == username;});
    }

    bool playerExistsByUUID(const std::string_view& uuid) {
        const auto& players = *Minecraft::Players.load();
        return std::any_of(players.begin(), players.end(), [&uuid](const Player& player)  { return player.getUUID() == uuid;});
    }
}