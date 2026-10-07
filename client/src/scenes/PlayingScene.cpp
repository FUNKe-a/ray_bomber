#include "PlayingScene.hpp"
#include "GameSession.hpp"
#include "MenuScene.hpp"
#include "Protocol.hpp"

#include <raylib.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
#include <iostream>

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

PlayingScene::PlayingScene(
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

std::unique_ptr<Scene> PlayingScene::update()
{
    if (session.getState() == GameSession::State::Failed ||
        session.getState() == GameSession::State::Disconnected)
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

    if (session.getState() != GameSession::State::Joined)
    {
        return nullptr;
    }
    
    const Game& game = session.getGame();
    const Player* localPlayer = game.getLocalPlayer();

    if (localPlayer && IsKeyPressed(KEY_R))
    {
        session.setReady(!localPlayer->isReady());
        std::cout << "Sent request is " << (!localPlayer->isReady() ? "ready" : "not ready") << std::endl;
    }

    // Arrow keys remain available for the existing movement demo. There is
    // no match-start/phase message in these schemas yet.
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

void PlayingScene::render() const
{
    renderer.render(session.getGame());
    drawLobby();
}

void PlayingScene::drawLobby() const
{
    const Game& game = session.getGame();

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
        const std::string label = name +
            (game.hasJoinedLobby() && id == game.getLocalPlayerId() ? " (you)" : "") +
            (participant->isReady() ? " - READY" : " - not ready");
        DrawText(label.c_str(), 18, y, 20,
                 participant->isReady() ? DARKGREEN : DARKGRAY);
        y += 24;
    }
}
