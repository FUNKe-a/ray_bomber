#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;

class MenuScene final : public Scene
{
public:
    explicit MenuScene(GameSession& session);

    MenuScene(
        GameSession& session,
        std::string serverAddress,
        std::string port,
        std::string errorMessage,
        std::string playerName = "Player"
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
    bool validateConnectionDetails();

    GameSession& session;
    std::string serverAddress;
    std::string port;
    std::string errorMessage;
    std::string playerName;
    InputField activeField = InputField::ServerAddress;
};
