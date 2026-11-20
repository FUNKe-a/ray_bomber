#include "SettingsScene.hpp"

#include "ConnectionSettings.hpp"
#include "SceneFactory.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <string>
#include <memory>
#include <utility>
#include <cctype>
#include <algorithm>
#include <array>

namespace
{
    Rectangle settingsRectangle()
    {
        constexpr float width = 460.0f;
        constexpr float height = 360.0f;

        return {
            (GetScreenWidth() - width) / 2.0f,
            (GetScreenHeight() - height) / 2.0f,
            width,
            height
        };
    }

    struct SettingsLayout
    {
        Rectangle nameField;
        Rectangle addressField;
        Rectangle portField;
        Rectangle undoButton;
        Rectangle backButton;
    };

    SettingsLayout makeSettingsLayout(Rectangle panel)
    {
        constexpr float horizontalPadding = 32.0f;
        constexpr float rowSpacing = 90.0f;
        constexpr float controlHeight = 48.0f;
        constexpr float columnGap = 16.0f;
        constexpr float portWidth = 64.0f;
        constexpr float undoWidth = 220.0f;

        const float left = panel.x + horizontalPadding;
        const float contentWidth = panel.width - 2.0f * horizontalPadding;
        
        const float nameY = panel.y + 100.0f;
        const float connectionY = nameY + rowSpacing;
        const float buttonY = connectionY + rowSpacing;

        SettingsLayout layout{};
        layout.nameField = {left, nameY, contentWidth, controlHeight};
        
        const float addressWidth = contentWidth - columnGap - portWidth;
        const float portX = left + addressWidth + columnGap;
        layout.addressField = {left, connectionY, addressWidth, controlHeight};
        layout.portField = {portX, connectionY, portWidth, controlHeight};

        const float backWidth = contentWidth - undoWidth - columnGap;
        const float backX = left + undoWidth + columnGap;
        layout.undoButton = {left, buttonY, undoWidth, controlHeight};
        layout.backButton = {backX, buttonY, backWidth, controlHeight};

        return layout;
    }

    SettingsField nextSettingsField(SettingsField current, bool backwards)
    {
        constexpr std::array order{
            SettingsField::PlayerName,
            SettingsField::ServerAddress,
            SettingsField::Port
        };

        const auto it = std::find(order.begin(), order.end(), current);

        if (it == order.end())
            return order.front();

        const auto index = static_cast<std::size_t>(it - order.begin());

        if (backwards)
            return index == 0 ? order.back() : order[index - 1];

        return order[(index + 1) % order.size()];
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
    const SettingsLayout layout = makeSettingsLayout(panelBounds());

    if (UI::isClicked(layout.backButton) ||
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
            layout.undoButton,
            commandHistory.canUndo()
        ))
    {
        while (GetCharPressed() > 0){}
        undoLastEdit();
        return std::nullopt;
    }

    if (UI::isClicked(layout.addressField))
        activeField = SettingsField::ServerAddress;
    else if (UI::isClicked(layout.portField))
        activeField = SettingsField::Port;
    else if (UI::isClicked(layout.nameField))
        activeField = SettingsField::PlayerName;
    else if (!controlDown && IsKeyPressed(KEY_TAB))
    {
        const bool backwards = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

        activeField = nextSettingsField(activeField, backwards);
    }

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
    const SettingsLayout layout = makeSettingsLayout(panel);

    const Color text{240, 240, 240, 255};

    UI::drawText(
        "Settings",
        panel.x + 32.0f,
        panel.y + 24.0f,
        32.0f,
        text
    );

    const auto theme = UI::darkTheme();

    drawFieldLabel(layout.nameField, "PLAYER NAME");
    UI::drawTextField(
        layout.nameField,
        settings.playerName,
        activeField == SettingsField::PlayerName,
        theme
    );

    drawFieldLabel(layout.addressField, "SERVER ADDRESS");
    UI::drawTextField(
        layout.addressField,
        settings.serverAddress,
        activeField == SettingsField::ServerAddress,
        theme
    );

    drawFieldLabel(layout.portField, "PORT");
    UI::drawTextField(
        layout.portField,
        settings.port,
        activeField == SettingsField::Port,
        theme
    );

    UI::drawButton(
        layout.undoButton,
        "UNDO  [Ctrl+Z]",
        commandHistory.canUndo(),
        UI::amberButtonTheme()
    );

    UI::drawButton(
        layout.backButton,
        "BACK TO MENU  [Esc]",
        true,
        UI::greenButtonTheme()
    );
}