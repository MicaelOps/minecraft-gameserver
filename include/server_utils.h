//
// Created by Micael Cossa on 27/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_UTILS_H
#define MINECRAFTSERVER_SERVER_UTILS_H

#include <iostream>
#include <mutex>
#include <array>
#include <string_view>



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
    OBFUSCATED,
    BOLD,
    STRIKETHROUGH,
    UNDERLINE,
    ITALIC,
    RESET,
    COUNT
};

inline constexpr std::array<std::string_view, std::to_underlying(COLOUR::COUNT)> colourCodes = {
        "§0","§1","§2","§3","§4","§5","§6","§7","§8","§9",
        "§a","§b","§c","§d","§e","§f","§g","§h","§i","§j",
        "§m","§n","§p","§q","§s","§t","§u","§k","§l","§m",
        "§n","§o","§r"
};
inline constexpr std::string_view getColour(COLOUR colour) {

    auto index = std::to_underlying(colour);
    return (index < std::size(colourCodes)) ? colourCodes[index] : "";
}

void printTime();

template<typename T>
void print(T value) {
    std::cout << value;
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
    static std::mutex logMutex;
    std::lock_guard<std::mutex> lock(logMutex);

    printTime();
    print(args...);
    std::cout << "\n"; // only one newline
}

#endif //MINECRAFTSERVER_SERVER_UTILS_H
