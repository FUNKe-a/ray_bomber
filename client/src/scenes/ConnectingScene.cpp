#include "ConnectingScene.hpp"

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


ConnectingScene::ConnectingScene(GameSession& session): session(session)
{
}

std::optional<SceneTransition> ConnectingScene::update()
{
    if (UI::isClicked(cancelButtonRectangle()) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, {}};
    }

    const auto state = session.getState();

    if (state == GameSession::State::Joined)
    {
        return SceneTransition{SceneType::Lobby, {}};
    }

    if (state == GameSession::State::Failed)
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, "Unable to connect to the server."};
    }

    if (state == GameSession::State::Disconnected)
    {
        SceneTransition{SceneType::Menu, "Disconnected from server."};
    }

    return std::nullopt;
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
