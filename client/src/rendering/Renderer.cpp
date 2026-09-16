#include "Renderer.hpp"

void Renderer::render(const Game& game)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    drawMap(game.getMap());
    drawPlayer(game.getPlayer());

    EndDrawing();
}

void Renderer::drawMap(const Map& map)
{
    for (int y = 0; y < Map::Height; ++y)
    {
        for (int x = 0; x < Map::Width; ++x)
        {
            int screenX = x * TileSize;
            int screenY = y * TileSize;

            if (map.getTile(x, y) == Map::Tile::Wall)
            {
                DrawRectangle(
                    screenX,
                    screenY,
                    TileSize,
                    TileSize,
                    DARKGRAY
                );
            }
            else
            {
                DrawRectangle(
                    screenX,
                    screenY,
                    TileSize,
                    TileSize,
                    LIGHTGRAY
                );
            }

            DrawRectangleLines(
                screenX,
                screenY,
                TileSize,
                TileSize,
                GRAY
            );
        }
    }
}

void Renderer::drawPlayer(const Player& player)
{
    int screenX = player.getX() * TileSize;
    int screenY = player.getY() * TileSize;

    DrawRectangle(
        screenX + 8,
        screenY + 8,
        TileSize - 16,
        TileSize - 16,
        BLUE
    );
}