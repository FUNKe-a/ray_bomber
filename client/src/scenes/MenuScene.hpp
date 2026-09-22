#pragma once

#include "ApplicationState.hpp"

#include <memory>
#include <string>

class Client;

class MenuScene final : public ApplicationState
{
public:
    explicit MenuScene(Client& client);

    MenuScene(
        Client& client,
        std::string serverAddress,
        std::string port,
        std::string errorMessage
    );

    std::unique_ptr<ApplicationState> update() override;
    void render() const override;

private:
    enum class InputField
    {
        ServerAddress,
        Port
    };

    void handleTextInput();
    bool validateConnectionDetails();

    Client& client;
    std::string serverAddress;
    std::string port;
    std::string errorMessage;
    InputField activeField = InputField::ServerAddress;
};
