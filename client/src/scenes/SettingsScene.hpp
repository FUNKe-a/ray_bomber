#pragma once

#include "Scene.hpp"

#include <memory>

struct ConnectionSettings;

class SettingsScene final : public Scene
{
public:
    SettingsScene(ConnectionSettings& settings);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    enum class InputField
    {
        ServerAddress,
        Port,
        PlayerName
    };

    void handleTextInput();

    ConnectionSettings& settings;

    InputField activeField = InputField::ServerAddress;
};