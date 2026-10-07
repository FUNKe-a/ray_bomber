#pragma once

#include "Scene.hpp"
#include "Game.hpp"
#include "GameRenderer.hpp"

#include <memory>
#include <string>

class GameSession;

class PlayingScene final : public Scene
{
public:
    explicit PlayingScene(
        GameSession& session,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    static constexpr int TileSize = 48;

    void drawLobby() const;

    GameSession& session;
    GameRenderer renderer;

    std::string serverAddress;
    std::string port;
    std::string playerName;
};
