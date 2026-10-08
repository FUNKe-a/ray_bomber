#pragma once

#include "Scene.hpp"

#include <memory>

class GameSession;
struct ConnectionSettings;

class SettingsScene final : public Scene
{
public:
    SettingsScene(
        GameSession& session,
        ConnectionSettings& settings
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    enum class InputField
    {
        ServerAddress,
        Port,
        PlayerName
    };

    void handleTextInput();

    GameSession& session;
    ConnectionSettings& settings;

    InputField activeField = InputField::ServerAddress;
};