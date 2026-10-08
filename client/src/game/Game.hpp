#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "GameEvent.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <optional>

class Game
{
public:
    explicit Game(std::string localName = "Player");
    void handleEvent(const GameEvent& event);
    const Map& getMap() const;
    const std::unordered_map<std::uint32_t, Player>& getPlayers() const;
    const Player* getPlayer(std::uint32_t id) const;
    const Player* getLocalPlayer() const;
    std::uint32_t getLocalPlayerId() const;
    bool hasJoinedLobby() const;
    std::optional<std::uint32_t> getLeaderId() const;
    bool isLocalPlayerLeader() const;

private:
    Player& ensurePlayer(std::uint32_t id);
    
    void apply(const LobbyJoined& event);
    void apply(const LobbyLeaderChanged& event);
    void apply(const PlayerJoined& event);
    void apply(const PlayerLeft& event);
    void apply(const PlayerReadyChanged& event);
    void apply(const PlayerSpawn& event);
    void apply(const PlayerMoved& event);
    void apply(const MatchPreparationStarted& event);
    void apply(const MatchSetupReceived& event);
    void apply(const MatchStarted& event);

    Map map;
    std::unordered_map<std::uint32_t, Player> players;
    std::string localName;
    std::uint32_t localPlayerId = 0;
    bool joinedLobby = false;
    std::optional<std::uint32_t> leaderId;
};
