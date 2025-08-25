#ifndef CORE_MINESERVER_MINECRAFT_H
#define CORE_MINESERVER_MINECRAFT_H

#include "plugins.h"
#include "concurrent_unordered_set.h"
#include "player.h"


#include <memory>
#include <set>


class Minecraft {
private:
    static Minecraft* instance;



    std::string motd = "This server uses a C++ Minecraft Server program!";
    std::set<PLUGIN> plugins;
    concurrent_unordered_set<std::shared_ptr<Player>> players;


public:

    Minecraft() = default;

    // Delete copy constructor and assignment
    Minecraft(const Minecraft&) = delete;
    Minecraft& operator=(const Minecraft&) = delete;

    static Minecraft* getServer() {
        if (instance == nullptr)
            instance = new Minecraft();
        return instance;
    }

};
#endif //CORE_MINESERVER_MINECRAFT_H
