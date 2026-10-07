#include "GameRenderer.hpp"

#include "Game.hpp"
#include "GameTypes.hpp"
#include "Map.hpp"
#include "Player.hpp"

#include <raylib.h>

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

void GameRenderer::render(const Game& game) const
{
    drawMap(game.getMap());
    drawPlayers(game);
}

void GameRenderer::drawMap(const Map& map) const
{
    for (int y = 0; y < Map::Height; ++y)
    {
        for (int x = 0; x < Map::Width; ++x)
        {
            const int screenX = x * TileSize;
            const int screenY = y * TileSize;

            const Color color =
                map.getTile(x, y) == Map::Tile::Wall
                    ? DARKGRAY
                    : LIGHTGRAY;

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
        if (!player.hasPosition())
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