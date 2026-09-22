#include "PlayingScene.hpp"

#include "Client.hpp"
#include "MenuScene.hpp"
#include "Protocol.hpp"

#include <raylib.h>

#include <array>
#include <cstdint>

namespace
{
    struct MovementBinding
    {
        KeyboardKey key;
        Protocol::Direction direction;
    };

    constexpr std::array<MovementBinding, 4> MovementBindings{{
        {KEY_UP, Protocol::Direction::Up},
        {KEY_DOWN, Protocol::Direction::Down},
        {KEY_LEFT, Protocol::Direction::Left},
        {KEY_RIGHT, Protocol::Direction::Right}
    }};
}

PlayingScene::PlayingScene(
    Client& client,
    std::string serverAddress,
    std::string port
)
    : client(client),
      serverAddress(std::move(serverAddress)),
      port(std::move(port))
{
}

std::unique_ptr<ApplicationState> PlayingScene::update()
{
    // receive messages
    Protocol::Message message;
    while (client.receive(message))
    {
        game.handleMessage(message);
    }

    // check connection
    if (!client.isConnected())
    {
        // std::string errorMessage = client.getError();

        // if (errorMessage.empty())
        // {
        // errorMessage = "Server closed the connection.";
        // }

        client.disconnect();

        return std::make_unique<MenuScene>(
            client,
            std::move(serverAddress),
            std::move(port),
            // std::move(errorMessage)
            "Server closed the connection."
        );
    }

    // handle input
    for (const MovementBinding& binding : MovementBindings)
    {
        if (IsKeyPressed(binding.key))
        {
            client.send(
                Protocol::createMoveRequest(binding.direction)
            );
        }
    }

    // dont change the scene
    return nullptr;
}

void PlayingScene::render() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawMap(game.getMap());
    drawPlayers();

    EndDrawing();
}

void PlayingScene::drawMap(const Map& map) const
{
    for (int y = 0; y < Map::Height; ++y)
    {
        for (int x = 0; x < Map::Width; ++x)
        {
            const int screenX = x * TileSize;
            const int screenY = y * TileSize;
            const Color color = map.getTile(x, y) == Map::Tile::Wall
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

void PlayingScene::drawPlayers() const
{
    const std::uint8_t localPlayerId = game.getLocalPlayerId();

    for (const auto& [playerId, player] : game.getPlayers())
    {
        drawPlayer(player, playerId == localPlayerId);
    }
}

void PlayingScene::drawPlayer(
    const Player& player,
    bool isLocalPlayer
) const
{
    const int screenX = player.getX() * TileSize;
    const int screenY = player.getY() * TileSize;
    const Color color = isLocalPlayer ? BLUE : RED;

    DrawRectangle(
        screenX + 8,
        screenY + 8,
        TileSize - 16,
        TileSize - 16,
        color
    );
}
