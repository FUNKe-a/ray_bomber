#pragma once

#include "raylib.h"

#include "../game/Game.hpp"

class Renderer
{
public:
    static constexpr int TileSize = 48;

    void render(const Game& game);

private:
    void drawMap(const Map& map);
    void drawPlayer(const Player& player);
};