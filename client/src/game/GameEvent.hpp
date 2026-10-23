#pragma once

#include "GameTypes.hpp"
#include "Map.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct PlayerInfo {
	std::uint32_t id;
	PlayerColor color;
	std::string username;
};

struct LobbyJoined {
    std::uint32_t id;
	std::uint32_t leader_id;
    PlayerColor color;
	std::vector<PlayerInfo> players;
};

struct PlayerJoined {
    std::uint32_t id;
    std::string name;
    PlayerColor color;
};

struct PlayerLeft {
    std::uint32_t id;
    std::string reason;
};

struct PlayerReadyChanged {
    std::uint32_t id;
    bool ready;
};

struct PlayerMoved {
    std::uint32_t id;
    int x;
    int y;
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
    std::uint64_t requestId = 0;
};

struct LobbyLeaderChanged
{
    std::uint32_t id;
};

struct MatchPreparationStarted {};

struct PlayerSpawn
{
    std::uint32_t id;
    int x;
    int y;
};

struct MatchSetupReceived
{
    std::vector<PlayerSpawn> spawns;
    std::uint64_t roundId = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<Map::Tile> terrain;
    RoundRules rules;
};

struct MatchStarted
{
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
};

using GameEvent = std::variant<
    LobbyJoined,
    LobbyLeaderChanged,
    PlayerJoined,
    PlayerLeft,
    PlayerReadyChanged,
    PlayerSpawn,
    PlayerMoved,
    MatchPreparationStarted,
    MatchSetupReceived,
    MatchStarted
>;
