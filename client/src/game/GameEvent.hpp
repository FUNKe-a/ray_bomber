#pragma once

#include "GameTypes.hpp"

#include <cstdint>
#include <string>
#include <variant>

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

struct PlayerSpawned {
    std::uint32_t id;
    int x;
    int y;
};

struct PlayerMoved {
    std::uint32_t id;
    int x;
    int y;
};

using GameEvent = std::variant<
    LobbyJoined,
    PlayerJoined,
    PlayerLeft,
    PlayerReadyChanged,
    PlayerSpawned,
    PlayerMoved
>;