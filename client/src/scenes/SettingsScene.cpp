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
    Rectangle fieldRectangle(float y)
    {
        return {
            48.0f,
            y,
            static_cast<float>(GetScreenWidth()) - 96.0f,
            50.0f
        };
    }

    Rectangle addressRectangle()
    {
        return fieldRectangle(155.0f);
    }

    Rectangle portRectangle()
    {
        return fieldRectangle(245.0f);
    }

    Rectangle nameRectangle()
    {
        return fieldRectangle(335.0f);
    }

    Rectangle backButtonRectangle()
    {
        return {
            GetScreenWidth() / 2.0f - 120.0f,
            430.0f,
            240.0f,
            55.0f
        };
    }

    Rectangle undoButtonRectangle()
    {
        return {
            GetScreenWidth() / 2.0f - 120.0f,
            500.0f,
            240.0f,
            45.0f
        };
    }

    void drawFieldLabel(Rectangle bounds, const char* text)
    {
        DrawText(
            text,
            static_cast<int>(bounds.x),
            static_cast<int>(bounds.y) - 28,
            20,
            GRAY
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
    UI::drawCenteredText(
        Rectangle{
            0.0f,
            55.0f,
            static_cast<float>(GetScreenWidth()),
            40.0f
        },
        "SETTINGS",
        40,
        DARKGRAY
    );

    const Rectangle addressRect = addressRectangle();
    const Rectangle portRect = portRectangle();
    const Rectangle nameRect = nameRectangle();

    drawFieldLabel(addressRect, "Server Address");
    UI::drawTextField(
        addressRect,
        settings.serverAddress,
        activeField == InputField::ServerAddress
    );

    drawFieldLabel(portRect, "Port");
    UI::drawTextField(
        portRect,
        settings.port,
        activeField == InputField::Port
    );

    drawFieldLabel(nameRect, "Player Name");
    UI::drawTextField(
        nameRect,
        settings.playerName,
        activeField == InputField::PlayerName
    );

    UI::drawButton(backButtonRectangle(), "BACK");

    UI::drawButton(undoButtonRectangle(), "UNDO (Ctrl+Z)", commandHistory.canUndo());
}