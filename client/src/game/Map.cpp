#include "Map.hpp"

#include <stdexcept>
#include <utility>

Map::Map(int width, int height, std::vector<Tile> cells)
    : width(width),
      height(height),
      cells(std::move(cells))
{
}

std::optional<Map> Map::fromBoard(const BoardData& board)
{
    // Local resource limit, independent of the protocol.
    constexpr std::uint32_t MaxDimension = 1024;

    if (board.width == 0 || board.height == 0 ||
        board.width > MaxDimension ||
        board.height > MaxDimension)
    {
        return std::nullopt;
    }

    const auto expectedCount =
        static_cast<std::uint64_t>(board.width) * board.height;

    if (board.cells.size() != expectedCount)
    {
        return std::nullopt;
    }

    for (const auto tile : board.cells)
    {
        switch (tile)
        {
            case Tile::Floor:
            case Tile::Wall:
            case Tile::Crate:
            case Tile::Bomb:
            case Tile::Explosion:
                break;

            default:
                return std::nullopt;
        }
    }

    return Map{
        static_cast<int>(board.width),
        static_cast<int>(board.height),
        board.cells
    };
}

int Map::getWidth() const { return width; }
int Map::getHeight() const { return height; }

bool Map::contains(int x, int y) const
{
    return x >= 0 && x < width &&
           y >= 0 && y < height;
}

bool Map::isWalkable(int x, int y) const
{
    return contains(x, y) && getTile(x, y) == Tile::Floor;
}

Tile Map::getTile(int x, int y) const
{
    if (!contains(x, y))
    {
        throw std::out_of_range("Tile coordinates are outside the map.");
    }

    const auto index = static_cast<std::size_t>(y) * width + x;

    return cells[index];
}

const std::vector<Tile>& Map::getCells() const
{
    return cells;
}

void Map::swap(Map& other) noexcept
{
    using std::swap;

    swap(width, other.width);
    swap(height, other.height);
    cells.swap(other.cells);
}