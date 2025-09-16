//
// Created by Micael Cossa on 02/06/2025.
//

#ifndef MINECRAFTSERVER_PLAYER_H
#define MINECRAFTSERVER_PLAYER_H


#include <string>


class Player {

private:
    std::string name, uuid;

public:

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] const std::string& getUUID() const;

};
template<> struct std::hash<Player> {
    std::size_t operator()(const Player& player) const noexcept {
        // Hash based on unique player properties (e.g., player ID or name)
        // Assuming Player has an id or name field:
        return std::hash<std::string>{}(player.getName()) ^
               std::hash<std::string>{}(player.getUUID());
    }
};

#endif //MINECRAFTSERVER_PLAYER_H
