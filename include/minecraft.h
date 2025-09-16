#ifndef CORE_MINESERVER_MINECRAFT_H
#define CORE_MINESERVER_MINECRAFT_H

#include "player.h"
#include "server_utils.h"

#include <unordered_set>
#include <memory>
#include <set>

// Conditional export/import macro
#ifdef BUILDING_CORE_MINESERVER_DLL
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

struct SERVER_INFO {

    std::string motd = std::string(getColour(COLOUR::BOLD)).append(getColour(COLOUR::GREEN)).append("This minecraft server uses C++! ");
    int maxPlayers = 30;

    void setMOTD(const std::string_view& newmotd) noexcept;

    [[nodiscard]] std::string getMOTD() const noexcept;
};


namespace Minecraft
{

    extern CORE_API SERVER_INFO ServerInformation;
    extern CORE_API const std::atomic<std::shared_ptr<std::unordered_set<Player>>> Players;

    SERVER_INFO getServerInformation();

    bool startup();

    bool playerExistsByUUID(const std::string_view &uuid);
    bool playerExistsByName(const std::string_view &username);

}

#endif //CORE_MINESERVER_MINECRAFT_H
