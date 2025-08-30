#include "minecraft.h"

#include <iostream>


void Minecraft::setMOTD(const std::string &newmotd) {
    info.motd = newmotd;
}

std::string Minecraft::getMOTD() const {
    return info.motd;
}

Minecraft &Minecraft::getServer() {
    static Minecraft instance;
    return instance;
}

SERVER_INFO Minecraft::getServerInfo() const {
    return info;
}

