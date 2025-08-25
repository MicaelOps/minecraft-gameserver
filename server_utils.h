//
// Created by Micael Cossa on 27/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_UTILS_H
#define MINECRAFTSERVER_SERVER_UTILS_H

#include <windows.h>
#include <iostream>
#include <array>


const std::array<std::string, 20> colourCodes = {};
enum class COLOUR : unsigned int {

    BLACK = 0,
    DARK_BLUE = 1,
    GREEN = 2,
    BLUE_GREEN = 3,
    RED = 4,
    PURPLE = 5,
    GOLDEN_YELLOW = 6,
    LIGHT_GRAY = 7,
    DARK_GRAY = 8,
    BLUE = 9,
    LIGHT_GREEN = 10, //A
    ELECTRIC_BLUE = 11,
    CRIMSON = 12,
    PINK = 13,
    YELLOW = 14;

    std::string getColour() {

    }
};

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
