#include "Map.hpp"

Map::Map()
{
    // Iš pradžių visas žemėlapis tuščias.
    for (int y = 0; y < Height; ++y)
    {
        for (int x = 0; x < Width; ++x)
        {
            tiles[y][x] = Tile::Empty;
        }
    }

    // Išorinės sienos.
    for (int x = 0; x < Width; ++x)
    {
        tiles[0][x] = Tile::Wall;
        tiles[Height - 1][x] = Tile::Wall;
    }

    for (int y = 0; y < Height; ++y)
    {
        tiles[y][0] = Tile::Wall;
        tiles[y][Width - 1] = Tile::Wall;
    }

    // Vidinės sienos.
    tiles[2][2] = Tile::Wall;
    tiles[2][3] = Tile::Wall;
    tiles[2][5] = Tile::Wall;
    tiles[2][6] = Tile::Wall;
    tiles[2][8] = Tile::Wall;
    tiles[2][9] = Tile::Wall;
    tiles[2][11] = Tile::Wall;
    tiles[2][12] = Tile::Wall;

    tiles[4][2] = Tile::Wall;
    tiles[4][3] = Tile::Wall;
    tiles[4][5] = Tile::Wall;
    tiles[4][6] = Tile::Wall;
    tiles[4][8] = Tile::Wall;
    tiles[4][9] = Tile::Wall;
    tiles[4][11] = Tile::Wall;
    tiles[4][12] = Tile::Wall;

    tiles[6][2] = Tile::Wall;
    tiles[6][4] = Tile::Wall;
    tiles[6][5] = Tile::Wall;
    tiles[6][6] = Tile::Wall;
    tiles[6][8] = Tile::Wall;
    tiles[6][9] = Tile::Wall;
    tiles[6][10] = Tile::Wall;
    tiles[6][12] = Tile::Wall;

    tiles[8][2] = Tile::Wall;
    tiles[8][3] = Tile::Wall;
    tiles[8][5] = Tile::Wall;
    tiles[8][6] = Tile::Wall;
    tiles[8][8] = Tile::Wall;
    tiles[8][9] = Tile::Wall;
    tiles[8][11] = Tile::Wall;
    tiles[8][12] = Tile::Wall;

    tiles[10][2] = Tile::Wall;
    tiles[10][3] = Tile::Wall;
    tiles[10][5] = Tile::Wall;
    tiles[10][6] = Tile::Wall;
    tiles[10][8] = Tile::Wall;
    tiles[10][9] = Tile::Wall;
    tiles[10][11] = Tile::Wall;
    tiles[10][12] = Tile::Wall;
}

bool Map::isWalkable(int x, int y) const
{
    if (x < 0 || x >= Width || y < 0 || y >= Height)
    {
        return false;
    }

    return tiles[y][x] == Tile::Empty;
}

Map::Tile Map::getTile(int x, int y) const
{
    return tiles[y][x];
}