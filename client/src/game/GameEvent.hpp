#pragma once

#include "GameTypes.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct LobbyJoined {
    std::uint32_t id;
    PlayerColor color;
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
};

struct MatchStarted {};

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