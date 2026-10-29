#include "GameRenderer.hpp"

#include "Game.hpp"
#include "GameTypes.hpp"
#include "Map.hpp"
#include "Player.hpp"

#include <raylib.h>

#include <algorithm>

namespace
{
    Color toRaylibColor(PlayerColor color, bool isLocalPlayer)
    {
        switch (color)
        {
            case PlayerColor::Red:    return RED;
            case PlayerColor::Green:  return GREEN;
            case PlayerColor::Blue:   return BLUE;
            case PlayerColor::Yellow: return YELLOW;
            case PlayerColor::Unknown:
                break;
        }

        // Preserve the existing fallback for unassigned colors.
        return isLocalPlayer ? BLUE : RED;
    }
}

void GameRenderer::render(
    const Game& game,
    Rectangle viewport
) const
{
    const auto& map = game.getMap();

    if (map.getWidth() <= 0 || map.getHeight() <= 0 ||
        viewport.width <= 0 || viewport.height <= 0)
        return;

    const float worldWidth = static_cast<float>(map.getWidth()) * TileSize;
    const float worldHeight = static_cast<float>(map.getHeight()) * TileSize;

    Camera2D camera{};

    camera.target = {
        worldWidth / 2.0f,
        worldHeight / 2.0f
    };

    camera.offset = {
        viewport.x + viewport.width / 2.0f,
        viewport.y + viewport.height / 2.0f
    };

    camera.zoom = std::min(
        viewport.width / worldWidth,
        viewport.height / worldHeight
    );

    BeginScissorMode(
        static_cast<int>(viewport.x),
        static_cast<int>(viewport.y),
        static_cast<int>(viewport.width),
        static_cast<int>(viewport.height)
    );

    BeginMode2D(camera);

    drawMap(map);
    drawPlayers(game);

    EndMode2D();
    EndScissorMode();
}

void GameRenderer::drawMap(const Map& map) const
{
    for (int y = 0; y < map.getHeight(); ++y)
    {
        for (int x = 0; x < map.getWidth(); ++x)
        {
            Color color = LIGHTGRAY;

            switch (map.getTile(x, y))
            {
                case Tile::Floor:
                    color = LIGHTGRAY;
                    break;

                case Tile::Wall:
                    color = DARKGRAY;
                    break;

                case Tile::Crate:
                    color = BROWN;
                    break;

                case Tile::Bomb:
                    color = BLACK;
                    break;

                case Tile::Explosion:
                    color = ORANGE;
                    break;
            }

            const int screenX = x * TileSize;
            const int screenY = y * TileSize;

            DrawRectangle(
                screenX,
                screenY,
                TileSize,
                TileSize,
                color
            );

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

void GameRenderer::drawPlayers(const Game& game) const
{
    const auto localPlayerId = game.getLocalPlayerId();

    for (const auto& [playerId, player] : game.getPlayers())
    {
        if (!player.hasPosition() || player.isEliminated())
            continue;

        const bool isLocalPlayer =
            game.hasJoinedLobby() && playerId == localPlayerId;

        drawPlayer(player, isLocalPlayer);
    }
}

void GameRenderer::drawPlayer(
    const Player& player,
    bool isLocalPlayer
) const
{
    const int screenX = player.getX() * TileSize;
    const int screenY = player.getY() * TileSize;

    DrawRectangle(
        screenX + PlayerPadding,
        screenY + PlayerPadding,
        TileSize - 2 * PlayerPadding,
        TileSize - 2 * PlayerPadding,
        toRaylibColor(player.getColor(), isLocalPlayer)
    );
}