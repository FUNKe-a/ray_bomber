#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "../protocol/Protocol.hpp"

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

    void handleMessage(const Protocol::Message& message);

    const Map& getMap() const;
    const Player& getPlayer() const;

private:
    Map map;
    Player player;
};