#include "MenuScene.hpp"

#include "ConnectingScene.hpp"
#include "ConnectionSettings.hpp"
#include "GameSession.hpp"
#include "SettingsScene.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <utility>

namespace
{
    Rectangle connectButtonRectangle()
    {
        return {
            GetScreenWidth() / 2.0f - 120.0f,
            240.0f,
            240.0f,
            55.0f
        };
    }

    Rectangle settingsButtonRectangle()
    {
        return {
            GetScreenWidth() / 2.0f - 120.0f,
            315.0f,
            240.0f,
            55.0f
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

std::unique_ptr<Scene> MenuScene::update()
{
    if (UI::isClicked(settingsButtonRectangle()))
    {
        return std::make_unique<SettingsScene>(session, settings);
    }

    if (UI::isClicked(connectButtonRectangle()))
    {
        const auto port =
            validateConnectionSettings(settings, errorMessage);

        if (!port)
        {
            return nullptr;
        }

        session.connect(
            settings.serverAddress,
            *port,
            settings.playerName
        );

        return std::make_unique<ConnectingScene>(session, settings);
    }

    return nullptr;
}

void MenuScene::render() const
{
    const float width = static_cast<float>(GetScreenWidth());

    UI::drawCenteredText(
        Rectangle{0.0f, 55.0f, width, 40.0f},
        "RAY BOMBER",
        40,
        DARKGRAY
    );

    const auto drawSetting = [&](float y, const std::string& label,
                                const std::string& value)
    {
        std::string text = label + (value.empty() ? "(not set)" : value);
        const int maxWidth = GetScreenWidth() - 96;

        // Keep long server addresses inside the menu.
        if (MeasureText(text.c_str(), 20) > maxWidth)
        {
            while (!text.empty() &&
                MeasureText((text + "...").c_str(), 20) > maxWidth)
            {
                text.pop_back();
            }

            text += "...";
        }

        UI::drawCenteredText(
            Rectangle{48.0f, y, width - 96.0f, 20.0f},
            text,
            20,
            UI::DefaultTheme.mutedText
        );
    };

    drawSetting(125.0f, "Server: ", settings.serverAddress);
    drawSetting(155.0f, "Port: ", settings.port);
    drawSetting(185.0f, "Player: ", settings.playerName);

    UI::drawButton(connectButtonRectangle(), "CONNECT");
    UI::drawButton(settingsButtonRectangle(), "SETTINGS");

    if (!errorMessage.empty())
    {
        UI::drawCenteredText(
            Rectangle{0.0f, 430.0f, width, 20.0f},
            errorMessage,
            20,
            RED
        );
    }
}