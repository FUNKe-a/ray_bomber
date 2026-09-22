#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "Protocol.hpp"

#include <cstdint>
#include <unordered_map>

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

    void handleMessage(const Protocol::Message& message);

    const Map& getMap() const;

    const std::unordered_map<std::uint8_t, Player>& getPlayers() const;
    
    const Player* getPlayer(std::uint8_t id) const;
    const Player* getLocalPlayer() const;
    
    std::uint8_t getLocalPlayerId() const;

private:
    void handleGreeting(const Protocol::Message& message);
    void handlePlayerJoined(const Protocol::Message& message);
    void handlePlayerLeft(const Protocol::Message& message);
    void handlePlayerMoved(const Protocol::Message& message);

    Map map;
    std::unordered_map<std::uint8_t, Player> players;

    std::uint8_t localPlayerId = 0;
    bool hasLocalPlayerId = false;
};
