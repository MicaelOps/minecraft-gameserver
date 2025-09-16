//
// Created by Micael Cossa on 16/09/2025.
//

#ifndef CORE_MINESERVER_NETWORKING_IMPORTS_H
#define CORE_MINESERVER_NETWORKING_IMPORTS_H

// Prevent common Windows header issues
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Include Winsock headers in correct order
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>

// Link required libraries
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#endif //CORE_MINESERVER_NETWORKING_IMPORTS_H
