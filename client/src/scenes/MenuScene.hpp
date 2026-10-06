#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class Client;

class MenuScene final : public Scene
{
public:
    explicit MenuScene(Client& client);

    MenuScene(
        Client& client,
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

    Client& client;
    std::string serverAddress;
    std::string port;
    std::string errorMessage;
    std::string playerName;
    InputField activeField = InputField::ServerAddress;
};
