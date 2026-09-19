#include "Renderer.hpp"
#include <raylib.h>

void Renderer::render(const Game& game)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    drawMap(game.getMap());
    drawPlayers(game);

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

void Renderer::drawPlayers(const Game& game)
{
    const std::uint8_t localPlayerId = game.getLocalPlayerId();

    for (const auto& [playerId, player] : game.getPlayers())
        drawPlayer(player, playerId == localPlayerId);
}


void Renderer::drawPlayer(
    const Player& player,
    bool isLocalPlayer
)
{
    const int screenX =
        player.getX() * TileSize;

    const int screenY =
        player.getY() * TileSize;

    const Color color =
        isLocalPlayer ? BLUE : RED;

    DrawRectangle(
        screenX + 8,
        screenY + 8,
        TileSize - 16,
        TileSize - 16,
        color
    );
}
