#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;

class LobbyScene final : public Scene
{
public:
    explicit LobbyScene(
        GameSession& session,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    std::unique_ptr<Scene> returnToMenu(std::string error);

    GameSession& session;

    std::string serverAddress;
    std::string port;
    std::string playerName;
};
