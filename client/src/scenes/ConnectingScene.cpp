#include "ConnectingScene.hpp"

#include "MenuScene.hpp"
#include "LobbyScene.hpp"
#include "GameSession.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <memory>
#include <utility>
#include <string>

namespace 
{
    Rectangle cancelButtonRectangle()
    {
        return {
            GetScreenWidth() / 2.0f - 120.0f,
            315.0f,
            240.0f,
            55.0f
        };
    }
}


ConnectingScene::ConnectingScene(
    GameSession& session,
    ConnectionSettings& settings
)
    : session(session),
      settings(settings)
{
}

std::unique_ptr<Scene> ConnectingScene::update()
{
    if (UI::isClicked(cancelButtonRectangle()) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        session.disconnect();
        return std::make_unique<MenuScene>(session, settings, "");
    }

    const auto state = session.getState();

    if (state == GameSession::State::Joined)
    {
        return std::make_unique<LobbyScene>(session, settings);
    }

    if (state == GameSession::State::Failed)
    {
        session.disconnect();

        return std::make_unique<MenuScene>(
            session,
            settings,
            "Unable to connect to the server."
        );
    }

    if (state == GameSession::State::Disconnected)
    {
        return std::make_unique<MenuScene>(
            session,
            settings,
            "Disconnected from server."
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
            220.0f,
            static_cast<float>(GetScreenWidth()),
            30.0f
        },
        text,
        30,
        DARKGRAY
    );

    UI::drawButton(cancelButtonRectangle(), "CANCEL");
}