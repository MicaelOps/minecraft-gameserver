#include "minecraft.h"

#include <iostream>


void Minecraft::setMOTD(const std::string &newmotd) {
    motd = newmotd;
}

std::string Minecraft::getMOTD() {
    return motd;
}

Minecraft &Minecraft::getServer() {
    static Minecraft instance;
    return instance;
}

