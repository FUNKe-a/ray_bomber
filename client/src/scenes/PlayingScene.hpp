#pragma once

#include "Scene.hpp"
#include "Game.hpp"

#include <memory>
#include <string>

class Client;

class PlayingScene final : public Scene
{
public:
    explicit PlayingScene(
        Client& client,
        std::string serverAddress,
        std::string port,
        std::string playerName
    );

    std::unique_ptr<Scene> update() override;
    void render() const override;

private:
    static constexpr int TileSize = 48;

    void drawMap(const Map& map) const;
    void drawPlayers() const;
    void drawPlayer(const Player& player, bool isLocalPlayer) const;
    void drawLobby() const;

    Client& client;
    Game game;

    std::string serverAddress;
    std::string port;
    std::string playerName;
    double joinStartedAt = 0.0;
};
