#include "Map.hpp"

#include <stdexcept>
#include <utility>

bool Map::validDimensions(std::uint32_t width, std::uint32_t height)
{
    return width > 0 && height > 0 &&
           width <= MaxDimension && height <= MaxDimension &&
           static_cast<std::uint64_t>(width) * height <= MaxCells;
}

Map::Map(std::uint32_t newWidth, std::uint32_t newHeight, std::vector<Tile> newCells)
    : width(newWidth), height(newHeight), cells(std::move(newCells))
{
    if (!validDimensions(width, height) ||
        cells.size() != static_cast<std::uint64_t>(width) * height)
        throw std::invalid_argument("Invalid board dimensions or cell count.");

    for (const auto tile : cells)
    {
        if (tile != Tile::Empty && tile != Tile::Wall && tile != Tile::Crate)
            throw std::invalid_argument("Invalid terrain.");
    }
}

std::uint32_t Map::getWidth() const { return width; }
std::uint32_t Map::getHeight() const { return height; }

bool Map::contains(int x, int y) const
{
    return x >= 0 && y >= 0 &&
           static_cast<std::uint32_t>(x) < width &&
           static_cast<std::uint32_t>(y) < height;
}

bool Map::isWalkable(int x, int y) const
{
    return contains(x, y) && getTile(x, y) == Tile::Empty;
}

Map::Tile Map::getTile(int x, int y) const
{
    if (!contains(x, y))
        throw std::out_of_range("Cell is outside the board.");
    return cells[static_cast<std::size_t>(y) * width + x];
}

const std::vector<Map::Tile>& Map::getCells() const { return cells; }
