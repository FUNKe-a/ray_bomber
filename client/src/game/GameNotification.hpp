#pragma once

#include "GameTypes.hpp"

#include <cstdint>
#include <variant>

struct WorldSnapshotApplied
{
    std::uint64_t roundId;
    std::uint64_t serverTick;
};

struct PlayerEliminationApplied
{
    std::uint64_t roundId;
    std::uint64_t serverTick;
    std::uint32_t playerId;
    EliminationReason reason;
};

struct GameplayActionRejected
{
    std::uint64_t roundId;
    std::uint64_t requestId;
    ActionRejectionReason reason;
};

using GameNotification = std::variant<
    WorldSnapshotApplied,
    PlayerEliminationApplied,
    GameplayActionRejected
>;