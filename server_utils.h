//
// Created by Micael Cossa on 27/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_UTILS_H
#define MINECRAFTSERVER_SERVER_UTILS_H

#include <windows.h>
#include <iostream>
#include <array>

enum class COLOUR : unsigned int {
    BLACK,
    DARK_BLUE,
    DARK_GREEN,
    DARK_AQUA,
    DARK_RED,
    DARK_PURPLE,
    GOLD,
    GRAY,
    DARK_GRAY,
    BLUE,
    GREEN,
    AQUA,
    RED,
    LIGHT_PURPLE,
    YELLOW,
    WHITE,
    MINECOIN_GOLD,
    MATERIAL_QUARTZ,
    MATERIAL_IRON,
    MATERIAL_NETHERITE,
    MATERIAL_REDSTONE,
    MATERIAL_COPPER,
    MATERIAL_GOLD,
    MATERIAL_EMERALD,
    MATERIAL_DIAMOND,
    MATERIAL_LAPIS,
    MATERIAL_AMETHYST,
    COUNT
};
constexpr std::array<std::string_view, static_cast<size_t>(COLOUR::COUNT)> colourCodes = {
        "§0", // BLACK
        "§1", // DARK_BLUE
        "§2", // DARK_GREEN
        "§3", // DARK_AQUA
        "§4", // DARK_RED
        "§5", // DARK_PURPLE
        "§6", // GOLD
        "§7", // GRAY
        "§8", // DARK_GRAY
        "§9", // BLUE
        "§a", // GREEN
        "§b", // AQUA
        "§c", // RED
        "§d", // LIGHT_PURPLE
        "§e", // YELLOW
        "§f", // WHITE
        "§g", // MINECOIN_GOLD
        "§h", // MATERIAL_QUARTZ
        "§i", // MATERIAL_IRON
        "§j", // MATERIAL_NETHERITE
        "§m", // MATERIAL_REDSTONE
        "§n", // MATERIAL_COPPER
        "§p", // MATERIAL_GOLD
        "§q", // MATERIAL_EMERALD
        "§s", // MATERIAL_DIAMOND
        "§t", // MATERIAL_LAPIS
        "§u"  // MATERIAL_AMETHYST
};
constexpr std::string_view getColour(const COLOUR colour) {

    auto colourIndex = static_cast<unsigned int>(colour);

    if(colourIndex >= colourCodes.size())
        return "";

    return colourCodes[colourIndex];
}


// Base case for recursion
template<typename T>
void print(T value) {
    std::cout << value << "\n";
}

// Recursive case
template<typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);
}

// Info logger
template<typename... Args>
void printInfo(Args... args) {
    SYSTEMTIME localTime;
    GetLocalTime(&localTime);
    std::cout << "[" << localTime.wHour << ":" << localTime.wMinute << ":" << localTime.wSecond << "] [INFO] ";
    print(args...);
}

#endif //MINECRAFTSERVER_SERVER_UTILS_H
