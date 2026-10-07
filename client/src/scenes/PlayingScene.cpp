#include "PlayingScene.hpp"

#include "Client.hpp"
#include "MenuScene.hpp"
#include "Protocol.hpp"

// Capture the protocol enum values before raylib defines its color macros.
// Macros expand even in qualified names such as player::RED.
namespace
{
    constexpr player::Color PlayerRed = player::RED;
    constexpr player::Color PlayerGreen = player::GREEN;
    constexpr player::Color PlayerBlue = player::BLUE;
    constexpr player::Color PlayerYellow = player::YELLOW;
}


#include <raylib.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
    struct MovementBinding
    {
        KeyboardKey key;
        player::MoveRequest::Direction direction;
    };

    constexpr std::array<MovementBinding, 4> MovementBindings{{
        {KEY_UP, player::MoveRequest::UP},
        {KEY_DOWN, player::MoveRequest::DOWN},
        {KEY_LEFT, player::MoveRequest::LEFT},
        {KEY_RIGHT, player::MoveRequest::RIGHT}
    }};
}

PlayingScene::PlayingScene(
    Client& client,
    std::string serverAddress,
    std::string port,
    std::string playerName
)
    : client(client),
      game(playerName),
      serverAddress(std::move(serverAddress)),
      port(std::move(port)),
      playerName(std::move(playerName)),
      joinStartedAt(GetTime())
{
}

std::unique_ptr<ApplicationState> PlayingScene::update()
{
    // receive messages
    Envelope message;
    while (client.receive(message))
    {
        game.handleMessage(message);
    }

    // check connection
    if (!client.isConnected() ||
        (!game.hasJoinedLobby() && GetTime() - joinStartedAt > 10.0))
    {
        std::string errorMessage = client.getError();
        if (errorMessage.empty())
            errorMessage = client.isConnected()
                ? "No lobby response received within 10 seconds."
                : "Server closed the connection.";
        client.disconnect();

        return std::make_unique<MenuScene>(
            client,
            std::move(serverAddress),
            std::move(port),
            std::move(errorMessage),
            std::move(playerName)
        );
    }

    // Only send gameplay/lobby requests after the join response.
    if (!game.hasJoinedLobby()) return nullptr;
    const Player* localPlayer = game.getLocalPlayer();
    if (localPlayer && IsKeyPressed(KEY_R)){
        client.send(Protocol::createReadyRequest(!localPlayer->isReady()));
        std::cout << "Sent a createReadyRequest" << std::endl;
    }

    // Arrow keys remain available for the existing movement demo. There is
    // no match-start/phase message in these schemas yet.
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
    drawLobby();

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
    const std::uint32_t localPlayerId = game.getLocalPlayerId();

    for (const auto& [playerId, player] : game.getPlayers())
    {
        if (player.hasPosition())
            drawPlayer(player, game.hasJoinedLobby() && playerId == localPlayerId);
    }
}

void PlayingScene::drawPlayer(
    const Player& player,
    bool isLocalPlayer
) const
{
    const int screenX = player.getX() * TileSize;
    const int screenY = player.getY() * TileSize;
    Color color = isLocalPlayer ? BLUE : RED;
    switch (player.getColor())
    {
        case PlayerRed: color = RED; break;
        case PlayerGreen: color = GREEN; break;
        case PlayerBlue: color = BLUE; break;
        case PlayerYellow: color = YELLOW; break;
        default: break;
    }

    DrawRectangle(
        screenX + 8,
        screenY + 8,
        TileSize - 16,
        TileSize - 16,
        color
    );

    const std::string name = player.getName().empty()
        ? "Player " + std::to_string(player.getId())
        : player.getName();

    const int fontSize = 16;
    const int nameWidth = MeasureText(name.c_str(), fontSize);
    const int nameX = screenX + (TileSize - nameWidth) / 2;
    const int nameY = screenY - fontSize - 2;

    DrawText(
        name.c_str(),
        nameX,
        nameY,
        fontSize,
        color
    );
}

void PlayingScene::drawLobby() const
{
    DrawRectangle(8, 8, 460, 52 + static_cast<int>(game.getPlayers().size()) * 24,
                  RAYWHITE);
    DrawText(game.hasJoinedLobby() ? "Lobby: R toggles ready | Arrows move"
                                 : "Joining lobby...", 18, 14, 20, DARKGRAY);
    // Stable presentation order despite the model using an unordered map.
    std::vector<std::uint32_t> ids;
    for (const auto& [id, participant] : game.getPlayers())
    {
        (void)participant;
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    int y = 42;
    for (const auto id : ids)
    {
        const Player* participant = game.getPlayer(id);
        const std::string name = participant->getName().empty()
            ? "Player " + std::to_string(id) : participant->getName();
        const std::string suffix =
            std::string(game.hasJoinedLobby() && id == game.getLocalPlayerId() ? " (you)" : "") +
            (participant->isReady() ? " - READY" : " - not ready");

        Color playerColor = DARKGRAY;
        switch (participant->getColor())
        {
        case PlayerRed: playerColor = RED; break;
        case PlayerGreen: playerColor = GREEN; break;
        case PlayerBlue: playerColor = BLUE; break;
        case PlayerYellow: playerColor = YELLOW; break;
        default: break;
        }

        DrawText(name.c_str(), 18, y, 20, playerColor);

        const int nameWidth = MeasureText(name.c_str(), 20);
        DrawText(suffix.c_str(), 18 + nameWidth, y, 20,
            participant->isReady() ? DARKGREEN : DARKGRAY);
        y += 24;
    }
}
