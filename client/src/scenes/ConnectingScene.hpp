#pragma once

#include "ApplicationState.hpp"

#include <memory>
#include <string>

class Client;

class ConnectingScene final : public ApplicationState
{
public:
    ConnectingScene(
        Client& client,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<ApplicationState> update() override;
    void render() const override;

private:
    Client& client;
    std::string serverAddress;
    std::string port;
    std::string playerName;
};
