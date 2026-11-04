#pragma once

#include "GameTypes.hpp"

#include <cstdint>
#include <vector>
#include <optional>

class Map
{
public:
    // Until MatchSetup arrives, there is no arena.
    Map() = default;
    
    static std::optional<Map> fromBoard(const BoardData& board);

    int getWidth() const;
    int getHeight() const;

    bool contains(int x, int y) const;
    bool isWalkable(int x, int y) const;
    
    Tile getTile(int x, int y) const;
    const std::vector<Tile>& getCells() const;

    void swap(Map& other) noexcept;

private:
    Map(int width, int height, std::vector<Tile> cells);
    
    int width = 0;
    int height = 0;
    std::vector<Tile> cells;
};
