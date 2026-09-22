#pragma once

#include "ApplicationState.hpp"
#include "Game.hpp"

#include <memory>
#include <string>

class Client;

class PlayingScene final : public ApplicationState
{
public:
    explicit PlayingScene(
        Client& client,
        std::string serverAddress,
        std::string port    
    );

    std::unique_ptr<ApplicationState> update() override;
    void render() const override;

private:
    static constexpr int TileSize = 48;

    void drawMap(const Map& map) const;
    void drawPlayers() const;
    void drawPlayer(const Player& player, bool isLocalPlayer) const;

    Client& client;
    Game game;

    std::string serverAddress;
    std::string port;
};
