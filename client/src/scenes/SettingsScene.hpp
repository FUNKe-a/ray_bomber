#pragma once

#include "PanelScene.hpp"
#include "SettingsCommand.hpp"

struct ConnectionSettings;

class SettingsScene final : public PanelScene
{
public:
    SettingsScene(ConnectionSettings& settings);

    std::optional<SceneTransition> update() override;

private:
    enum class InputField
    {
        ServerAddress,
        Port,
        PlayerName
    };

    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    void handleTextInput(bool controlDown);
    void undoLastEdit();

    ConnectionSettings& settings;
    InputField activeField = InputField::PlayerName;
    SettingsCommandHistory commandHistory;
};