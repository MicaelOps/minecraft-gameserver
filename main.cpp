#include <iostream>

#include "server_connection.h"
#include "server_handler.h"
#include "server_utils.h"


bool close = false;
SERVER_CONNECTION serverConnection;

int main() {

    if(!startupServerNetwork(&serverConnection)) {
        printInfo("Unable to startup server connection. See errors above.");
        return -1;
    }

    setupPacketFactory();

    printInfo("Network setup completed. ");
    std::string end;

    while(!close) {
        std::cin >> end;
        close = end == "end";
    }


    std::cout << "Closing server...";
    closeServerConnections(&serverConnection);
    return 0;
}
