#pragma once

#include <cstdint>
#include <vector>

class Map
{
public:
    enum class Tile { Empty, Wall, Crate };

    // Until MatchSetup arrives, there is no arena.
    Map() = default;
    Map(std::uint32_t width, std::uint32_t height, std::vector<Tile> cells);

    std::uint32_t getWidth() const;
    std::uint32_t getHeight() const;
    bool contains(int x, int y) const;
    bool isWalkable(int x, int y) const;
    Tile getTile(int x, int y) const;
    const std::vector<Tile>& getCells() const;

    // Local resource limits, not a restriction in the wire protocol.
    static constexpr std::uint32_t MaxDimension = 64;
    static constexpr std::uint64_t MaxCells = 64 * 64;
    static bool validDimensions(std::uint32_t width, std::uint32_t height);

private:
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<Tile> cells;
};
