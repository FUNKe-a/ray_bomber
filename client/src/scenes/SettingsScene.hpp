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
    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    void handleTextInput(bool controlDown);
    void undoLastEdit();

    ConnectionSettings& settings;
    SettingsField activeField = SettingsField::PlayerName;
    SettingsCommandHistory commandHistory;
};