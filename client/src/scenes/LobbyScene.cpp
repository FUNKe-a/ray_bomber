#include "LobbyScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"
#include "MenuScene.hpp"

#include <raylib.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
    struct MovementBinding {
        KeyboardKey key;
        Direction direction;
    };

    constexpr std::array<MovementBinding, 4> MovementBindings{{
        {KEY_UP, Direction::Up},
        {KEY_DOWN, Direction::Down},
        {KEY_LEFT, Direction::Left},
        {KEY_RIGHT, Direction::Right}
    }};
}

LobbyScene::LobbyScene(
    GameSession& session,
    std::string serverAddress,
    std::string port,
    std::string playerName
)
    : session(session),
      serverAddress(std::move(serverAddress)),
      port(std::move(port)),
      playerName(std::move(playerName))
{
}

std::unique_ptr<Scene> LobbyScene::update()
{
    const auto state = session.getState();

    if (state == GameSession::State::Failed ||
        state == GameSession::State::Disconnected)
    {
        std::string error = session.getError();

        if (error.empty())
            error = "Disconnected from server.";

        return std::make_unique<MenuScene>(
            session,
            std::move(serverAddress),
            std::move(port),
            std::move(error),
            std::move(playerName)
        );
    }

    if (state != GameSession::State::Joined)
    {
        return nullptr;
    }
    
    const Game& game = session.getGame();
    const Player* localPlayer = game.getLocalPlayer();

    if (localPlayer && IsKeyPressed(KEY_R))
    {
        session.setReady(!localPlayer->isReady());
    }

    for (const MovementBinding& binding : MovementBindings)
    {
        if (IsKeyPressed(binding.key))
        {
            session.move(binding.direction);
        }
    }

    // dont change the scene
    return nullptr;
}

void LobbyScene::render() const
{
    renderer.render(session.getGame());
    drawLobby();
}

void LobbyScene::drawLobby() const
{
    const Game& game = session.getGame();

    const int panelHeight =
        52 + static_cast<int>(game.getPlayers().size()) * 24;

    DrawRectangle(8, 8, 460, panelHeight, RAYWHITE);

    DrawText(
        "Lobby: R toggles ready | Arrows move",
        18,
        14,
        20,
        DARKGRAY
    );

    // Keep the display order stable.
    std::vector<std::uint32_t> ids;
    ids.reserve(game.getPlayers().size());

    for (const auto& [id, participant] : game.getPlayers())
    {
        ids.push_back(id);
    }

    std::sort(ids.begin(), ids.end());

    int y = 42;

    for (const auto id : ids)
    {
        const Player* participant = game.getPlayer(id);

        const std::string name =
            participant->getName().empty()
                ? "Player " + std::to_string(id)
                : participant->getName();

        const bool isLocalPlayer =
            game.hasJoinedLobby() &&
            id == game.getLocalPlayerId();

        const std::string label =
            name +
            (isLocalPlayer ? " (you)" : "") +
            (participant->isReady() ? " - READY" : " - not ready");

        DrawText(
            label.c_str(),
            18,
            y,
            20,
            participant->isReady() ? DARKGREEN : DARKGRAY
        );

        y += 24;
    }
}
