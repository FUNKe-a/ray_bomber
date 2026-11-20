#include "SettingsScene.hpp"

#include "ConnectionSettings.hpp"
#include "SceneFactory.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <string>
#include <memory>
#include <utility>
#include <cctype>
#include <cstddef>

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

    void erasePreviousWord(std::string & text)
    {
        while(!text.empty() && text.back() == ' ')
            text.pop_back();

        if (text.empty())
            return;
        
        const auto isWordCharacter = [](char ch){
            return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
        };

        const bool deletingWord = isWordCharacter(text.back());

        do text.pop_back();
        while (
            !text.empty() && 
            text.back() != ' ' && 
            isWordCharacter(text.back()) == deletingWord
        );
    }

    enum class DeleteAction
    {
        None, Character, PreviousWord
    };

    struct TextEdit
    {
        std::string typed;
        DeleteAction deletion = DeleteAction::None;
    };

    TextEdit readTextEdit(bool controlDown)
    {
        TextEdit edit;

        for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed()){
            if(!controlDown && ch >= 32 && ch <= 126)
                edit.typed.push_back(static_cast<char>(ch));
        }

        if(IsKeyPressed(KEY_BACKSPACE))
            edit.deletion = controlDown 
                ? DeleteAction::PreviousWord 
                : DeleteAction::Character;
        
        return edit;
    }

    bool isAddressCharacter(char ch)
    {
        return (ch >= '0' && ch <= '9') ||
               (ch >= 'a' && ch <= 'z') ||
               (ch >= 'A' && ch <= 'Z') ||
               ch == '.' || ch == '-' || ch == ':';
    }

    bool isPortCharacter(char ch)
    {
        return ch >= '0' && ch <= '9';
    }

    bool isNameCharacter(char ch)
    {
        return ch >= ' ' && ch <= '~';
    }

    struct FieldRules
    {
        std::size_t maxLength;
        bool (*accepts)(char);
    };

    std::string applyTextEdit(std::string value, const TextEdit& edit, FieldRules rules)
    {
        for (char ch : edit.typed){
            if(value.size() < rules.maxLength && rules.accepts(ch))
                value.push_back(ch);
        }

        switch (edit.deletion){
            case DeleteAction::Character:
                if(!value.empty()) value.pop_back();
                break;
            case DeleteAction::PreviousWord:
                erasePreviousWord(value);
                break;
            case DeleteAction::None:
                break;
        }

        return value;
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
        activeField = SettingsField::ServerAddress;
    else if (UI::isClicked(portRectangle()))
        activeField = SettingsField::Port;
    else if (UI::isClicked(nameRectangle()))
        activeField = SettingsField::PlayerName;

    handleTextInput(controlDown);
    return std::nullopt;
}

void SettingsScene::handleTextInput(bool controlDown)
{
    const TextEdit edit = readTextEdit(controlDown);

    if(edit.typed.empty() && edit.deletion == DeleteAction::None)
        return;

    const auto editField = [&](std::string& field, FieldRules rules)
    {
        std::string updated = applyTextEdit(field, edit, rules);

        if(updated == field)
            return;
        
        commandHistory.execute(
            activeField,
            std::make_unique<ChangeTextCommand>(field, std::move(updated))
        );
    };

    switch (activeField)
    {
        case SettingsField::PlayerName:
            editField(settings.playerName, {32, isNameCharacter});
            break;

        case SettingsField::ServerAddress:
            editField(settings.serverAddress, {255, isAddressCharacter});
            break;

        case SettingsField::Port:
            editField(settings.port, {5, isPortCharacter});
            break;
    }
}

void SettingsScene::undoLastEdit()
{
    if (const auto restoredField = commandHistory.undo())
    {
        activeField = *restoredField;
        TraceLog(LOG_INFO, "SETTINGS COMMAND: undo");
    }
}

Rectangle SettingsScene::panelBounds() const
{
    return settingsRectangle();
}

void SettingsScene::drawContent(Rectangle panel) const
{
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};

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
        activeField == SettingsField::PlayerName,
        theme
    );

    const auto address = addressRectangle();
    drawFieldLabel(address, "SERVER ADDRESS");

    UI::drawTextField(
        address,
        settings.serverAddress,
        activeField == SettingsField::ServerAddress,
        theme
    );

    const auto port = portRectangle();
    drawFieldLabel(port, "PORT");

    UI::drawTextField(
        port,
        settings.port,
        activeField == SettingsField::Port,
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