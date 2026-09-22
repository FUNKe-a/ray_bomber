#include "Game.hpp"

#include <iostream>

Game::Game()
    : map()
{
}

void Game::handleMessage(const Protocol::Message& message)
{
    switch (message.type)
    {
        case Protocol::MessageType::Greeting:
            handleGreeting(message);
            break;

        case Protocol::MessageType::PlayerJoined:
            handlePlayerJoined(message);
            break;

        case Protocol::MessageType::PlayerLeft:
            handlePlayerLeft(message);
            break;

        case Protocol::MessageType::PlayerMoved:
            handlePlayerMoved(message);
            break;

        default:
            break;
    }
}

void Game::handleGreeting(const Protocol::Message& message)
{
    const auto greeting = Protocol::deserializeGreeting(message);

    localPlayerId = greeting.id;
    hasLocalPlayerId = true;

    std::cout
        << "Received Greeting\n"
        << "  Local player ID: "
        << static_cast<int>(localPlayerId)
        << '\n';
}

void Game::handlePlayerJoined(const Protocol::Message& message)
{
    const auto joined = Protocol::deserializePlayerJoined(message);

    const auto [iterator, inserted] =
        players.emplace(
            joined.id,
            Player(
                joined.id,
                joined.startX,
                joined.startY
            )
        );

    if (!inserted)
    {
        iterator->second.setPosition(
            joined.startX,
            joined.startY
        );
    }

    std::cout
        << "Player joined\n"
        << "  ID: "
        << static_cast<int>(joined.id)
        << '\n'
        << "  Position: "
        << static_cast<int>(joined.startX)
        << ", "
        << static_cast<int>(joined.startY)
        << '\n';
}

void Game::handlePlayerLeft(const Protocol::Message& message)
{
    const auto left = Protocol::deserializePlayerLeft(message);

    players.erase(left.id);

    if(left.id == localPlayerId)
    {
        localPlayerId = 0;
        hasLocalPlayerId = false;
    }

    std::cout
        << "Player left\n"
        << "  ID: "
        << static_cast<int>(left.id)
        << '\n';
}

void Game::handlePlayerMoved(const Protocol::Message& message)
{
    const auto moved = Protocol::deserializePlayerMoved(message);

    auto iterator = players.find(moved.id);

    if (iterator == players.end())
    {
        std::cerr
            << "Received PlayerMoved for unknown player: "
            << static_cast<int>(moved.id)
            << '\n';

        return;
    }

    iterator->second.setPosition(
        moved.x,
        moved.y
    );

    std::cout
        << "Player moved\n"
        << "  ID: "
        << static_cast<int>(moved.id)
        << '\n'
        << "  Position: "
        << static_cast<int>(moved.x)
        << ", "
        << static_cast<int>(moved.y)
        << '\n';
}

const Map& Game::getMap() const
{
    return map;
}

const std::unordered_map<std::uint8_t, Player>& Game::getPlayers() const
{
    return players;
}

const Player* Game::getPlayer(std::uint8_t id) const
{
    const auto iterator = players.find(id);

    if (iterator == players.end())
    {
        return nullptr;
    }

    return &iterator->second;
}

const Player* Game::getLocalPlayer() const
{
    if (!hasLocalPlayerId)
    {
        return nullptr;
    }

    return getPlayer(localPlayerId);
}

std::uint8_t Game::getLocalPlayerId() const
{
    return localPlayerId;
}
