#include "ConnectingScene.hpp"

#include "GameSession.hpp"
#include "Controls.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <memory>
#include <utility>
#include <string>

namespace
{
    Rectangle loadingRectangle()
    {
        constexpr float width = 500.0f;
        constexpr float height = 300.0f;

        return {
            (GetScreenWidth() - width) / 2.0f,
            (GetScreenHeight() - height) / 2.0f,
            width,
            height
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

struct ConnectingScene::Controls
{
    bool cancelRequested = false;
    UI::Button cancel;
    UI::Form form;

    Controls() :
        cancel("CANCEL", UI::darkTheme(),
            [this] { cancelRequested = true; }),
        form{cancel}
    {
        cancel.setBounds(cancelButtonRectangle());
    }
};


ConnectingScene::ConnectingScene(GameSession& session)
    : session(session),
      controls(std::make_unique<Controls>())
{
}

ConnectingScene::~ConnectingScene() = default;

std::optional<SceneTransition> ConnectingScene::update()
{
    controls->form.update();

    if (controls->cancelRequested || IsKeyPressed(KEY_ESCAPE))
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

Rectangle ConnectingScene::panelBounds() const
{
    return loadingRectangle();
}

void ConnectingScene::drawContent(Rectangle panel) const
{
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};

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

    controls->form.draw();
}