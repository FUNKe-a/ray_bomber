#include "Game.hpp"

#include <iostream>

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

void Game::handleMessage(const Protocol::Message& message)
{
    switch (message.type)
    {
    case Protocol::MessageType::PlayerMoved:
    {
        const auto moved =
            Protocol::deserializePlayerMoved(message);

        player.setPosition(
            moved.x,
            moved.y
        );

        break;
    }

    case Protocol::MessageType::Greeting:
    {
        const auto greeting =
            Protocol::deserializeGreeting(message);

        std::cout
            << "Received Greeting\n"
            << "  ID: "
            << static_cast<int>(greeting.id)
            << '\n';

        break;
    }

    case Protocol::MessageType::PlayerJoined:
    {
        const auto joined =
            Protocol::deserializePlayerJoined(message);

        std::cout
            << "Received PlayerJoined\n"
            << "  ID: "
            << static_cast<int>(joined.id)
            << '\n'
            << "  Position: "
            << static_cast<int>(joined.startX)
            << ", "
            << static_cast<int>(joined.startY)
            << '\n';

        break;
    }

    case Protocol::MessageType::PlayerLeft:
    {
        const auto left =
            Protocol::deserializePlayerLeft(message);

        std::cout
            << "Received PlayerLeft\n"
            << "  ID: "
            << static_cast<int>(left.id)
            << '\n';

        break;
    }

    default:
        break;
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
