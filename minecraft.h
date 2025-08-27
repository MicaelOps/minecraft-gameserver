#ifndef CORE_MINESERVER_MINECRAFT_H
#define CORE_MINESERVER_MINECRAFT_H

#include "plugins.h"
#include "concurrent_unordered_set.h"
#include "player.h"
#include "server_utils.h"

#include <memory>
#include <set>

class Minecraft {
private:

    const std::set<PLUGIN> plugins;
    const concurrent_unordered_set<std::shared_ptr<Player>> players;

    std::string motd = "§b§oThis server uses a C++ Minecraft Server program!";

    // Private constructor prevents external instantiation
    Minecraft() = default;

public:


    // Delete copy constructor and assignment
    Minecraft(const Minecraft&) = delete;
    Minecraft& operator=(const Minecraft&) = delete;

    static Minecraft& getServer();

    void setMOTD(const std::string& newmotd);
    std::string getMOTD();
};
#endif //CORE_MINESERVER_MINECRAFT_H
