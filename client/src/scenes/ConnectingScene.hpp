#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class Client;

class ConnectingScene final : public Scene
{
public:
    ConnectingScene(
        Client& client,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    Client& client;
    std::string serverAddress;
    std::string port;
    std::string playerName;
};
