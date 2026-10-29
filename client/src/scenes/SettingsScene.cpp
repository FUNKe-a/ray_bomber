#include "SettingsScene.hpp"

#include "ConnectionSettings.hpp"
#include "SceneFactory.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <string>
#include <memory>
#include <utility>

namespace
{
    Rectangle settingsRectangle()
    {
        return {
            (GetScreenWidth() - 640.0f) / 2.0f,
            (GetScreenHeight() - 552.0f) / 2.0f,
            640.0f,
            552.0f
        };
    }

    Rectangle fieldRectangle(float offset)
    {
        const auto panel = settingsRectangle();

        return {
            panel.x + 32.0f,
            panel.y + offset,
            panel.width - 64.0f,
            48.0f
        };
    }

    Rectangle nameRectangle()
    {
        return fieldRectangle(154.0f);
    }

    Rectangle addressRectangle()
    {
        return fieldRectangle(244.0f);
    }

    Rectangle portRectangle()
    {
        return fieldRectangle(334.0f);
    }

    Rectangle undoButtonRectangle()
    {
        const auto panel = settingsRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 424.0f,
            220.0f,
            48.0f
        };
    }

    Rectangle backButtonRectangle()
    {
        const auto panel = settingsRectangle();

        return {
            panel.x + 268.0f,
            panel.y + 424.0f,
            panel.width - 300.0f,
            48.0f
        };
    }

    void drawFieldLabel(Rectangle bounds, const char* text)
    {
        UI::drawText(
            text,
            bounds.x,
            bounds.y - 26.0f,
            14.0f,
            Color{165, 165, 165, 255}
        );
    }

    void discardTextInput()
    {
        while (GetCharPressed() > 0)
        {
        }
    }
}

SettingsScene::SettingsScene(ConnectionSettings& settings): settings(settings)
{
}

std::optional<SceneTransition> SettingsScene::update()
{
    if (UI::isClicked(backButtonRectangle()) ||
        IsKeyPressed(KEY_ESCAPE))
    {
        return SceneTransition{SceneType::Menu, {}};
    }

    const bool controlDown =
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);

    const bool undoShortcut =
        controlDown && IsKeyPressed(KEY_Z);

    if (undoShortcut ||
        UI::isClicked(
            undoButtonRectangle(),
            commandHistory.canUndo()
        ))
    {
        discardTextInput();
        undoLastEdit();
        return std::nullopt;
    }

    if (UI::isClicked(addressRectangle()))
    {
        activeField = InputField::ServerAddress;
    }
    else if (UI::isClicked(portRectangle()))
    {
        activeField = InputField::Port;
    }
    else if (UI::isClicked(nameRectangle()))
    {
        activeField = InputField::PlayerName;
    }

    if (controlDown)
    {
        discardTextInput();
        return std::nullopt;
    }

    handleTextInput();
    return std::nullopt;
}

void SettingsScene::handleTextInput()
{
    std::string& target =
        activeField == InputField::ServerAddress
            ? settings.serverAddress
            : activeField == InputField::Port
                ? settings.port
                : settings.playerName;

    std::string editedValue = target;

    int character = GetCharPressed();

    while (character > 0)
    {
        bool allowed = false;
        std::size_t maxLength = 0;

        switch (activeField)
        {
            case InputField::ServerAddress:
                allowed =
                    (character >= '0' && character <= '9') ||
                    (character >= 'a' && character <= 'z') ||
                    (character >= 'A' && character <= 'Z') ||
                    character == '.' ||
                    character == '-' ||
                    character == ':';

                maxLength = 255;
                break;

            case InputField::Port:
                allowed = character >= '0' && character <= '9';
                maxLength = 5;
                break;

            case InputField::PlayerName:
                allowed = character >= 32 && character <= 126;
                maxLength = 32;
                break;
        }

        if (allowed && editedValue.size() < maxLength)
            editedValue += static_cast<char>(character);

        character = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !editedValue.empty())
        editedValue.pop_back();

    // Ignore rejected input and edits that leave the value unchanged.
    if (editedValue == target)
        return;

    commandHistory.execute(
        std::make_unique<ChangeTextCommand>(
            target,
            std::move(editedValue)
        )
    );

    TraceLog(LOG_INFO, "SETTINGS COMMAND: change text");
}

void SettingsScene::undoLastEdit()
{
    if (commandHistory.undo())
        TraceLog(LOG_INFO, "SETTINGS COMMAND: undo");
}

void SettingsScene::render() const
{
    const Color background{20, 20, 20, 255};
    const Color panelColor{29, 29, 29, 255};
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};

    DrawRectangle(
        0, 0,
        GetScreenWidth(), GetScreenHeight(),
        background
    );

    const auto panel = settingsRectangle();

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

    UI::drawText(
        "CONNECTION",
        panel.x + 32.0f,
        panel.y + 24.0f,
        14.0f,
        muted
    );

    UI::drawText(
        "Settings",
        panel.x + 32.0f,
        panel.y + 48.0f,
        32.0f,
        text
    );

    UI::drawText(
        "Choose your name and the server to connect to.",
        panel.x + 32.0f,
        panel.y + 91.0f,
        16.0f,
        muted
    );

    const auto theme = UI::darkTheme();

    const auto name = nameRectangle();
    drawFieldLabel(name, "PLAYER NAME");

    UI::drawTextField(
        name,
        settings.playerName,
        activeField == InputField::PlayerName,
        theme
    );

    const auto address = addressRectangle();
    drawFieldLabel(address, "SERVER ADDRESS");

    UI::drawTextField(
        address,
        settings.serverAddress,
        activeField == InputField::ServerAddress,
        theme
    );

    const auto port = portRectangle();
    drawFieldLabel(port, "PORT");

    UI::drawTextField(
        port,
        settings.port,
        activeField == InputField::Port,
        theme
    );

    UI::drawButton(
        undoButtonRectangle(),
        "UNDO  [Ctrl+Z]",
        commandHistory.canUndo(),
        UI::amberButtonTheme()
    );

    UI::drawButton(
        backButtonRectangle(),
        "BACK TO MENU  [Esc]",
        true,
        UI::greenButtonTheme()
    );

    UI::drawCenteredText(
        Rectangle{
            panel.x + 32.0f,
            panel.y + 492.0f,
            panel.width - 64.0f,
            24.0f
        },
        "Edits apply immediately. Undo is available until you leave.",
        14,
        muted
    );
}