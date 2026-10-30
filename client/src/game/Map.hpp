#pragma once

#include <array>

class Map
{
public:
    static constexpr int Width = 15;
    static constexpr int Height = 13;

    enum class Tile
    {
        Empty,
        Wall
    };

    Map();

    bool isWalkable(int x, int y) const;
    Tile getTile(int x, int y) const;

private:
    std::array<std::array<Tile, Width>, Height> tiles;
};