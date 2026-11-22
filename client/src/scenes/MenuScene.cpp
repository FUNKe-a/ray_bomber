#include "MenuScene.hpp"

#include "ConnectionSettings.hpp"
#include "GameSession.hpp"
#include "Controls.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <utility>

namespace
{
    Rectangle menuRectangle()
    {
        constexpr float width = 500.0f;
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

        drawRow(13.0f, "PLAYER", settings.value(SettingsField::PlayerName));
        drawRow(48.0f, "SERVER", settings.value(SettingsField::ServerAddress));
        drawRow(83.0f, "PORT", settings.value(SettingsField::Port));
    }
}

struct MenuScene::Controls
{
    MenuLayout layout;
    UI::Button connect;
    UI::Button settings;
    UI::Form form;

    Controls(MenuScene& scene, Rectangle panel)
        : layout(makeMenuLayout(panel)),
          connect("CONNECT", UI::greenButtonTheme(),
                    [&scene] { scene.requestConnect(); }),
          settings("SETTINGS", UI::amberButtonTheme(),
                    [&scene] { scene.pendingTransition =
                        SceneTransition{SceneType::Settings, {}};
                    }),
          form{connect, settings}
    {
        connect.setBounds(layout.connectButton);
        settings.setBounds(layout.settingsButton);
    }
};

MenuScene::~MenuScene() = default;

MenuScene::MenuScene(
    GameSession& session,
    ConnectionSettings& settings,
    std::string errorMessage
)
    : session(session),
      settings(settings),
      errorMessage(std::move(errorMessage))
{
    controls = std::make_unique<Controls>(*this, panelBounds());
}

std::optional<SceneTransition> MenuScene::update()
{
    controls->form.update();
    return std::exchange(pendingTransition, std::nullopt);
}

void MenuScene::requestConnect()
{
    const auto port = validateConnectionSettings(settings, errorMessage);

    if (!port)
        return;

    session.connect(
        settings.value(SettingsField::ServerAddress),
        *port,
        settings.value(SettingsField::PlayerName)
    );

    pendingTransition = SceneTransition{SceneType::Connecting, {}};
}

Rectangle MenuScene::panelBounds() const
{
    return menuRectangle();
}

void MenuScene::drawContent(Rectangle) const
{
    const auto& layout = controls->layout;

    UI::drawCenteredText(layout.title, "RAY BOMBER", 40, MenuText);

    drawConnectionCard(layout.connectionCard, settings);
    controls->form.draw();

    if (!errorMessage.empty())
    {
        UI::drawFittedText(
            layout.errorMessage, errorMessage, 16.0f, MenuError
        );
    }
}