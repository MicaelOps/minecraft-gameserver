#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>   // std::move
#include <functional>

// ----------------------
// Location
// ----------------------
struct Location {
    int x{};
    int y{};
    int z{};
    std::string worldname;

    // Getters
    [[nodiscard]] int getX() const noexcept { return x; }
    [[nodiscard]] int getY() const noexcept { return y; }
    [[nodiscard]] int getZ() const noexcept { return z; }
    [[nodiscard]] std::string_view getWorldName() const noexcept { return worldname; }

    // Setters
    void setX(int newX) noexcept { x = newX; }
    void setY(int newY) noexcept { y = newY; }
    void setZ(int newZ) noexcept { z = newZ; }
    void setWorldName(std::string newName) { worldname = std::move(newName); }

    // Needed for use as key in unordered_map
    bool operator==(const Location& other) const noexcept {
        return x == other.x && y == other.y && z == other.z && worldname == other.worldname;
    }
};

// Custom hash for Location
struct LocationHash {
    std::size_t operator()(const Location& loc) const noexcept {
        std::size_t h1 = std::hash<int>{}(loc.x);
        std::size_t h2 = std::hash<int>{}(loc.y);
        std::size_t h3 = std::hash<int>{}(loc.z);
        std::size_t h4 = std::hash<std::string>{}(loc.worldname);
        return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1) ^ (h4 << 2);
    }
};

// ----------------------
// Block
// ----------------------
struct Block {
    Location loc;
    int id{};
    short data{};
};

// ----------------------
// World
// ----------------------
class World {

public:

    void update()  {

    }
    // Insert or update a block at a given location
    void setBlock(const Location& loc, int id, short data) {
        blocks[loc] = Block{loc, id, data};
    }

    // Try to get a block at a given location
    [[nodiscard]] const Block* getBlock(const Location& loc) const {
        if (auto it = blocks.find(loc); it != blocks.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Check if block exists at a location
    [[nodiscard]] bool hasBlock(const Location& loc) const {
        return blocks.find(loc) != blocks.end();
    }

private:
    std::string name;
    std::unordered_map<Location, Block, LocationHash> blocks;
};
