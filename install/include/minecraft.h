#ifndef CORE_MINESERVER_MINECRAFT_H
#define CORE_MINESERVER_MINECRAFT_H

#include "plugins.h"
#include "concurrent_unordered_set.h"
#include "player.h"
#include "server_utils.h"

#include <memory>
#include <set>


struct SERVER_INFO {

    std::string motd = std::string(getColour(COLOUR::AQUA)).append(getColour(COLOUR::ITALIC)).append("This server uses a C++ Minecraft Server program!");
    int maxPlayers = 20;
};
class Minecraft {
private:

    const std::set<PLUGIN> plugins;
    const concurrent_unordered_set<std::shared_ptr<Player>> players;

    SERVER_INFO info;

    // Private constructor prevents external instantiation
    Minecraft() = default;

public:


    // Delete copy constructor and assignment
    Minecraft(const Minecraft&) = delete;
    Minecraft& operator=(const Minecraft&) = delete;

    static Minecraft& getServer();

    void setMOTD(const std::string& newmotd) noexcept;

    std::string getMOTD() const;
    SERVER_INFO getServerInfo() const;
};
#endif //CORE_MINESERVER_MINECRAFT_H
