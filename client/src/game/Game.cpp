#include "Game.hpp"

Game::Game()
    : map(),
      player(1, 1)
{
}

void Game::movePlayer(Direction direction)
{
    int newX = player.getX();
    int newY = player.getY();

    switch (direction)
    {
        case Direction::Up:
            --newY;
            break;

        case Direction::Down:
            ++newY;
            break;

        case Direction::Left:
            --newX;
            break;

        case Direction::Right:
            ++newX;
            break;
    }

    if (map.isWalkable(newX, newY))
    {
        player.setPosition(newX, newY);
    }
}

const Map& Game::getMap() const
{
    return map;
}

const Player& Game::getPlayer() const
{
    return player;
}