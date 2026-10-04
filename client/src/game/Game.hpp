#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "envelope.pb.h"
#include <cstdint>
#include <string>
#include <unordered_map>

class Game
{
public:
    explicit Game(std::string localName = "Player");
    void handleMessage(const Envelope& message);
    const Map& getMap() const;
    const std::unordered_map<std::uint32_t, Player>& getPlayers() const;
    const Player* getPlayer(std::uint32_t id) const;
    const Player* getLocalPlayer() const;
    std::uint32_t getLocalPlayerId() const;
    bool hasJoinedLobby() const;

private:
    Player& ensurePlayer(std::uint32_t id);
    void handleJoinResponse(const player::JoinLobbyResponse& response);
    void handlePlayerEvent(const player::PlayerEvent& event);
    void handleMovement(const player::PlayerMovement& movement);
    Map map;
    std::unordered_map<std::uint32_t, Player> players;
    std::string localName;
    std::uint32_t localPlayerId = 0;
    bool joinedLobby = false;
};
