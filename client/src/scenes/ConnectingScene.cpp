#include "ConnectingScene.hpp"

#include "MenuScene.hpp"
#include "LobbyScene.hpp"
#include "GameSession.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <memory>
#include <utility>

ConnectingScene::ConnectingScene(
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

std::unique_ptr<Scene> ConnectingScene::update()
{
    if (session.getState() == GameSession::State::Joined)
    {
        return std::make_unique<LobbyScene>(
            session, 
            std::move(serverAddress),
            std::move(port),
            std::move(playerName)
        );
    }

    if (session.getState() == GameSession::State::Failed)
    {
        return std::make_unique<MenuScene>(
            session,
            std::move(serverAddress),
            std::move(port),
            session.getError(),
            std::move(playerName)
        );
    }

    return nullptr;
}

void ConnectingScene::render() const
{
    const char* text =
    session.getState() == GameSession::State::Joining
        ? "Joining lobby..."
        : "Connecting...";

    
    UI::drawCenteredText(
        Rectangle{
            0.0f,
            0.0f,
            static_cast<float>(GetScreenWidth()),
            static_cast<float>(GetScreenHeight())
        },
        text,
        30,
        DARKGRAY
    );
}