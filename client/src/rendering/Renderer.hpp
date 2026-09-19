#pragma once

#include "Game.hpp"

class Renderer
{
public:
    static constexpr int TileSize = 48;

    void render(const Game& game);

private:
    void drawMap(const Map& map);
    void drawPlayers(const Game& game);
    void drawPlayer(const Player& player, bool isLocalPlayer);
};
