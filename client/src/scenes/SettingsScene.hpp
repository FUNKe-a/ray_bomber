#pragma once

#include "PanelScene.hpp"
#include "SettingsCommand.hpp"
#include "ConnectionSettings.hpp"

#include <optional>
#include <memory>
#include <array>

class SettingsScene final : public PanelScene
{
public:
    SettingsScene(ConnectionSettings& settings);
    ~SettingsScene() override;

    std::optional<SceneTransition> update() override;

private:
    Rectangle panelBounds() const override;
    void drawContent(Rectangle panel) const override;

    void updateControlBounds();
    void updateControlAvailability();
    void undoLastEdit();
    void restoreOpeningSettings();
    void requestBackToMenu();

    ConnectionSettings& settings;
    const ConnectionSettings::Memento openingSettings;
    struct Controls;
    std::unique_ptr<Controls> controls;
    SettingsCommandHistory commandHistory;
    bool backRequested = false;

    std::array<bool, 3> showFieldError{};
    std::optional<SettingsField> focusedSettingsField() const;
};