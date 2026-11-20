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
        constexpr float width = 460.0f;
        constexpr float height = 384.0f;

        return {
            (GetScreenWidth() - width) / 2.0f,
            (GetScreenHeight() - height) / 2.0f,
            width,
            height
        };
    }

    struct MenuLayout
    {
        Rectangle title;
        Rectangle connectionCard;
        Rectangle connectButton;
        Rectangle settingsButton;
        Rectangle errorMessage;
    };

    MenuLayout makeMenuLayout(Rectangle panel)
    {
        constexpr float horizontalPadding = 32.0f;

        const float left = panel.x + horizontalPadding;
        const float width = panel.width - 2.0f * horizontalPadding;

        MenuLayout layout{};
        layout.title          = {left, panel.y + 16.0f,  width, 54.0f};
        layout.connectionCard = {left, panel.y + 86.0f,  width, 122.0f};
        layout.connectButton  = {left, panel.y + 238.0f, width, 48.0f};
        layout.settingsButton = {left, panel.y + 300.0f, width, 48.0f};
        layout.errorMessage   = {left, panel.y + 350.0f, width, 34.0f};
        return layout;
    }

    constexpr Color MenuText{240, 240, 240, 255};
    constexpr Color MenuMuted{165, 165, 165, 255};
    constexpr Color MenuCard{42, 42, 42, 255};
    constexpr Color MenuError{239, 168, 107, 255};

    void drawConnectionCard(Rectangle bounds, const ConnectionSettings& settings)
    {
        DrawRectangleRounded(bounds, 0.1f, 8, MenuCard);

        const auto drawRow = [&](float offset,
                                const char* label,
                                const std::string& value)
        {
            const float rowY = bounds.y + offset;

            UI::drawText(label,
                bounds.x + 16.0f,
                rowY + 3.0f,
                14.0f,
                MenuMuted
            );

            const Rectangle valueBounds{
                bounds.x + 96.0f,
                rowY,
                bounds.width - 112.0f,
                25.0f
            };

            UI::drawFittedText(
                valueBounds,
                value.empty() ? "(not set)" : value,
                17.0f,
                MenuText
            );
        };

        drawRow(13.0f, "PLAYER", settings.playerName);
        drawRow(48.0f, "SERVER", settings.serverAddress);
        drawRow(83.0f, "PORT", settings.port);
    }

    void drawMenuButtons(const MenuLayout& layout)
    {
        UI::drawButton(
            layout.connectButton, "CONNECT", true, UI::greenButtonTheme()
        );

        UI::drawButton(
            layout.settingsButton, "SETTINGS", true, UI::amberButtonTheme()
        );
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
    const MenuLayout layout = makeMenuLayout(panelBounds());

    if (UI::isClicked(layout.settingsButton))
        return SceneTransition{SceneType::Settings, {}};

    if (UI::isClicked(layout.connectButton))
    {
        const auto port = validateConnectionSettings(settings, errorMessage);

        if (!port)
            return std::nullopt;

        session.connect(settings.serverAddress, *port, settings.playerName);
        return SceneTransition{SceneType::Connecting, {}};
    }

    return std::nullopt;
}

Rectangle MenuScene::panelBounds() const
{
    return menuRectangle();
}

void MenuScene::drawContent(Rectangle panel) const
{
    const MenuLayout layout = makeMenuLayout(panel);

    UI::drawCenteredText(layout.title, "RAY BOMBER", 40, MenuText);

    drawConnectionCard(layout.connectionCard, settings);
    drawMenuButtons(layout);

    if (!errorMessage.empty())
    {
        UI::drawFittedText(
            layout.errorMessage, errorMessage, 15.0f, MenuError
        );
    }
}