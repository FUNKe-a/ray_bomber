#include "SettingsScene.hpp"

#include "ConnectionSettings.hpp"
#include "SceneFactory.hpp"
#include "Controls.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <string>
#include <memory>
#include <utility>

namespace
{
    Rectangle settingsRectangle()
    {
        constexpr float width = 500.0f;
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
        Rectangle restoreButton;
        Rectangle backButton;
    };

    SettingsLayout makeSettingsLayout(Rectangle panel)
    {
        constexpr float horizontalPadding = 32.0f;
        constexpr float rowSpacing = 90.0f;
        constexpr float controlHeight = 48.0f;
        constexpr float columnGap = 16.0f;
        constexpr float portWidth = 64.0f;

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

        const float buttonWidth = (contentWidth - 2.0f * columnGap) / 3.0f;
        const float buttonStep = buttonWidth + columnGap;
        const float backX = left + 2 * buttonStep;

        layout.undoButton = {left, buttonY, buttonWidth, controlHeight};
        layout.restoreButton = {left + buttonStep, buttonY, buttonWidth, controlHeight};
        layout.backButton = {backX, buttonY, buttonWidth, controlHeight};

        return layout;
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

    void discardTextInput()
    {
        while (GetCharPressed() > 0)
        {
            // Drain queued characters for this frame.
        }
    } 

    UI::LineInput makeSettingsInput(
        ConnectionSettings& settings,
        SettingsCommandHistory& history,
        SettingsField field,
        std::size_t maxLength,
        UI::LineInput::AcceptCharacter accepts
    )
    {
        return UI::LineInput{
            maxLength,
            accepts,
            [&settings, field]() -> const std::string&
            {
                return settings.value(field);
            },
            [&settings, &history, field](std::string edited)
            {
                history.execute(
                    field,
                    std::make_unique<ChangeTextCommand>(
                        settings, field, std::move(edited)
                    )
                );
            }
        };
    }
}

SettingsScene::SettingsScene(ConnectionSettings& settings)
    : settings(settings),
      openingSettings(settings.save()),
      controls(std::make_unique<Controls>(*this))
{
    updateControlBounds();
    updateControlAvailability();
}

struct SettingsScene::Controls
{
    UI::LineInput name;
    UI::LineInput address;
    UI::LineInput port;
    UI::Button undo;
    UI::Button restore;
    UI::Button back;
    UI::Form form;

    explicit Controls(SettingsScene& scene)
        : name(makeSettingsInput(
              scene.settings, scene.commandHistory,
              SettingsField::PlayerName, 32, isNameCharacter)),
          address(makeSettingsInput(
              scene.settings, scene.commandHistory,
              SettingsField::ServerAddress, 255, isAddressCharacter)),
          port(makeSettingsInput(
              scene.settings, scene.commandHistory,
              SettingsField::Port, 5, isPortCharacter)),
          undo("UNDO", UI::amberButtonTheme(),
               [&scene] { scene.undoLastEdit(); }),
          restore("RESTORE", UI::darkTheme(),
                  [&scene] { scene.restoreOpeningSettings(); }),
          back("BACK TO MENU", UI::greenButtonTheme(),
               [&scene] { scene.requestBackToMenu(); }),
          form{name, address, port, undo, restore, back}
    {
    }
};

std::optional<SettingsField> SettingsScene::focusedSettingsField() const
{
    if (controls->form.hasFocus(controls->name))
        return SettingsField::PlayerName;

    if (controls->form.hasFocus(controls->address))
        return SettingsField::ServerAddress;

    if (controls->form.hasFocus(controls->port))
        return SettingsField::Port;

    return std::nullopt;
}

SettingsScene::~SettingsScene() = default;

std::optional<SceneTransition> SettingsScene::update()
{
    updateControlBounds();

    const auto previousField = focusedSettingsField();

    const bool controlDown =
        IsKeyDown(KEY_LEFT_CONTROL) ||
        IsKeyDown(KEY_RIGHT_CONTROL);

    if (IsKeyPressed(KEY_ESCAPE)){
        discardTextInput();
        requestBackToMenu();
    } else if (controlDown && IsKeyPressed(KEY_Z))
    {
        discardTextInput();
        undoLastEdit();
    }
    else
    {
        controls->form.update();
    }

    if (backRequested)
        return SceneTransition{SceneType::Menu, {}};

    updateControlAvailability();

    const auto currentField = focusedSettingsField();

    if (previousField != currentField)
    {
        if (previousField)
            showFieldError[static_cast<std::size_t>(*previousField)] = true;

        if (currentField)
            showFieldError[static_cast<std::size_t>(*currentField)] = false;
    }

    return std::nullopt;
}

Rectangle SettingsScene::panelBounds() const
{
    return settingsRectangle();
}

void SettingsScene::drawContent(Rectangle panel) const
{
    const Color text{240, 240, 240, 255};

    UI::drawText(
        "Settings",
        panel.x + 32.0f,
        panel.y + 24.0f,
        32.0f,
        text
    );

    drawFieldLabel(controls->name.bounds(), "PLAYER NAME");
    drawFieldLabel(controls->address.bounds(), "SERVER NAME");
    drawFieldLabel(controls->port.bounds(), "PORT");
    
    controls->form.draw();

    const Color errorColor{239, 168, 107, 255};
    const char* firstError = nullptr;

    const auto drawFieldError = [&](SettingsField field, const UI::LineInput& input)
    {
        if (!showFieldError[static_cast<std::size_t>(field)])
            return;

        if (const char* message = settingsFieldError(settings, field))
        {
            DrawRectangleLinesEx(input.bounds(), 2.0f, errorColor);

            if (!firstError)
                firstError = message;
        }
    };

    drawFieldError(SettingsField::PlayerName, controls->name);
    drawFieldError(SettingsField::ServerAddress, controls->address);
    drawFieldError(SettingsField::Port, controls->port);

    if (firstError){
        UI::drawCenteredText(
            Rectangle{
                panel.x + 32.0f,
                panel.y + panel.height - 28.0f,
                panel.width - 64.0f,
                24.0f
            },
            firstError,
            14,
            firstError ? errorColor : UI::DefaultTheme.mutedText

        );
    }
}

void SettingsScene::updateControlBounds()
{
    const auto layout = makeSettingsLayout(panelBounds());

    controls->name.setBounds(layout.nameField);
    controls->address.setBounds(layout.addressField);
    controls->port.setBounds(layout.portField);
    controls->undo.setBounds(layout.undoButton);
    controls->restore.setBounds(layout.restoreButton);
    controls->back.setBounds(layout.backButton);
}

void SettingsScene::updateControlAvailability()
{
    controls->undo.setEnabled(commandHistory.canUndo());
    controls->restore.setEnabled(settings.differsFrom(openingSettings));
}

void SettingsScene::undoLastEdit()
{
    const auto field = commandHistory.undo();
    if (!field)
        return;

    switch (*field)
    {
        case SettingsField::PlayerName:
            controls->form.focus(controls->name);
            break;

        case SettingsField::ServerAddress:
            controls->form.focus(controls->address);
            break;

        case SettingsField::Port:
            controls->form.focus(controls->port);
            break;
    }
}

void SettingsScene::restoreOpeningSettings()
{
    settings.restore(openingSettings);
    commandHistory.clear();
    controls->form.focus(controls->name);
}

void SettingsScene::requestBackToMenu()
{
    backRequested = true;
}