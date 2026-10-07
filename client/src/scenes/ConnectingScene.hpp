#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;

class ConnectingScene final : public Scene
{
public:
    ConnectingScene(
        GameSession& session,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    GameSession& session;
    std::string serverAddress;
    std::string port;
    std::string playerName;
};
