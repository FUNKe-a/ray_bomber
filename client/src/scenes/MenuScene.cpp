#include "MenuScene.hpp"

#include "ConnectionSettings.hpp"
#include "GameSession.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <utility>

namespace
{
    Rectangle menuRectangle()
    {
        return {
            (GetScreenWidth() - 560.0f) / 2.0f,
            (GetScreenHeight() - 520.0f) / 2.0f,
            560.0f,
            520.0f
        };
    }

    Rectangle connectButtonRectangle()
    {
        const auto panel = menuRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 322.0f,
            panel.width - 64.0f,
            50.0f
        };
    }

    Rectangle settingsButtonRectangle()
    {
        const auto panel = menuRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 384.0f,
            panel.width - 64.0f,
            46.0f
        };
    }
}

MenuScene::MenuScene(
    GameSession& session,
    ConnectionSettings& settings,
    std::string errorMessage
)
    : session(session),
      settings(settings),
      errorMessage(std::move(errorMessage))
{
}

std::optional<SceneTransition> MenuScene::update()
{
    if (UI::isClicked(settingsButtonRectangle()))
    {
       return SceneTransition(SceneType::Settings, {});
    }

    if (UI::isClicked(connectButtonRectangle()))
    {
        const auto port =
            validateConnectionSettings(settings, errorMessage);

        if (!port)
        {
            return std::nullopt;
        }

        session.connect(
            settings.serverAddress,
            *port,
            settings.playerName
        );

        return SceneTransition{SceneType::Connecting, {}};
    }

    return std::nullopt;
}

void MenuScene::render() const
{
    const Color background{20, 20, 20, 255};
    const Color panelColor{29, 29, 29, 255};
    const Color surface{42, 42, 42, 255};
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};

    DrawRectangle(
        0, 0,
        GetScreenWidth(), GetScreenHeight(),
        background
    );

    const auto panel = menuRectangle();

    DrawRectangleRounded(
        Rectangle{
            panel.x + 5.0f,
            panel.y + 7.0f,
            panel.width,
            panel.height
        },
        0.04f,
        12,
        Color{0, 0, 0, 65}
    );

    DrawRectangleRounded(panel, 0.04f, 12, panelColor);

    UI::drawCenteredText(
        Rectangle{
            panel.x + 32.0f,
            panel.y + 64.0f,
            panel.width - 64.0f,
            54.0f
        },
        "RAY BOMBER",
        40,
        text
    );

    const Rectangle connectionCard{
        panel.x + 32.0f,
        panel.y + 176.0f,
        panel.width - 64.0f,
        122.0f
    };

    DrawRectangleRounded(connectionCard, 0.1f, 8, surface);

    const auto drawSetting = [&](float offset,
                                 const char* label,
                                 const std::string& value)
    {
        UI::drawText(
            label,
            connectionCard.x + 16.0f,
            connectionCard.y + offset + 3.0f,
            14.0f,
            muted
        );

        UI::drawFittedText(
            Rectangle{
                connectionCard.x + 96.0f,
                connectionCard.y + offset,
                connectionCard.width - 112.0f,
                25.0f
            },
            value.empty() ? "(not set)" : value,
            17.0f,
            text
        );
    };

    drawSetting(13.0f, "PLAYER", settings.playerName);
    drawSetting(48.0f, "SERVER", settings.serverAddress);
    drawSetting(83.0f, "PORT", settings.port);

    UI::drawButton(
        connectButtonRectangle(),
        "CONNECT",
        true,
        UI::greenButtonTheme()
    );

    UI::drawButton(
        settingsButtonRectangle(),
        "SETTINGS",
        true,
        UI::amberButtonTheme()
    );

    if (!errorMessage.empty())
    {
        UI::drawFittedText(
            Rectangle{
                panel.x + 32.0f,
                panel.y + 450.0f,
                panel.width - 64.0f,
                34.0f
            },
            errorMessage,
            15.0f,
            Color{239, 168, 107, 255}
        );
    }
}