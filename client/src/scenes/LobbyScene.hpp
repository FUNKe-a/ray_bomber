#pragma once

#include "Scene.hpp"
#include "GameRenderer.hpp"

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
    void drawLobby() const;

    GameSession& session;
    GameRenderer renderer;

    std::string serverAddress;
    std::string port;
    std::string playerName;
};
