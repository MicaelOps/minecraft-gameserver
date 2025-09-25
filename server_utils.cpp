//
// Created by Micael Cossa on 27/07/2025.
//
#include "server_utils.h"
#include <iostream>
#include <windows.h>






void printTime() {
    SYSTEMTIME localTime;
    GetLocalTime(&localTime);
    std::cout << "["
              << localTime.wHour << ":"
              << localTime.wMinute << ":"
              << localTime.wSecond
              << "] [INFO] ";
}