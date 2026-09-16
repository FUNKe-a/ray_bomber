#pragma once

#include "Map.hpp"
#include "Player.hpp"

enum class Direction
{
    Up,
    Down,
    Left,
    Right
};

class Game
{
public:
    Game();

    void movePlayer(Direction direction);

    const Map& getMap() const;
    const Player& getPlayer() const;

private:
    Map map;
    Player player;
};
