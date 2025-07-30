//
// Created by Micael Cossa on 27/07/2025.
//

#ifndef MINECRAFTSERVER_SERVER_UTILS_H
#define MINECRAFTSERVER_SERVER_UTILS_H

#include <windows.h>
#include <iostream>

// Base case for recursion
template<typename T>
void print(T value) {
    std::cout << value << std::endl;
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
