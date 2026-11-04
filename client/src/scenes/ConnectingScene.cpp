#include "ConnectingScene.hpp"

#include "GameSession.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <memory>
#include <utility>
#include <string>

namespace
{
    Rectangle loadingRectangle()
    {
        return {
            (GetScreenWidth() - 480.0f) / 2.0f,
            (GetScreenHeight() - 300.0f) / 2.0f,
            480.0f,
            300.0f
        };
    }

    Rectangle cancelButtonRectangle()
    {
        const auto panel = loadingRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 220.0f,
            panel.width - 64.0f,
            48.0f
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
        std::string error = session.getError();

        if (error.empty())
            error = "Unable to connect to the server.";

        session.disconnect();

        return SceneTransition{SceneType::Menu, std::move(error)};
    }

    if (state == GameSession::State::Disconnected)
    {
        return SceneTransition{SceneType::Menu, "Disconnected from server."};
    }

    return std::nullopt;
}

void ConnectingScene::render() const
{
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};

    DrawRectangle(
        0, 0,
        GetScreenWidth(), GetScreenHeight(),
        Color{20, 20, 20, 255}
    );

    const auto panel = loadingRectangle();

    DrawRectangleRounded(
        panel,
        0.05f,
        12,
        Color{29, 29, 29, 255}
    );

    UI::drawCenteredText(
        Rectangle{
            panel.x + 32.0f,
            panel.y + 28.0f,
            panel.width - 64.0f,
            24.0f
        },
        "MULTIPLAYER",
        14,
        muted
    );

    const bool joining =
        session.getState() == GameSession::State::Joining;

    UI::drawCenteredText(
        Rectangle{
            panel.x + 32.0f,
            panel.y + 72.0f,
            panel.width - 64.0f,
            42.0f
        },
        joining ? "Joining lobby..." : "Connecting...",
        28,
        text
    );

    UI::drawCenteredText(
        Rectangle{
            panel.x + 32.0f,
            panel.y + 126.0f,
            panel.width - 64.0f,
            24.0f
        },
        joining
            ? "Waiting for the server to accept your join."
            : "Establishing a connection to the server.",
        15,
        muted
    );

    // Activity indicator, not a percentage of completion.
    const int activeDot =
        static_cast<int>(GetTime() * 3.0) % 3;

    for (int index = 0; index < 3; ++index)
    {
        DrawCircleV(
            Vector2{
                panel.x + panel.width / 2.0f +
                    (index - 1) * 22.0f,
                panel.y + 180.0f
            },
            4.0f,
            index == activeDot
                ? Color{112, 218, 160, 255}
                : Color{65, 65, 65, 255}
        );
    }

    UI::drawButton(
        cancelButtonRectangle(),
        "CANCEL  [Esc]",
        true,
        UI::darkTheme()
    );
}