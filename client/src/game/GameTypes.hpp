#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include <string>

enum class Tile
{
    Floor,
    Wall,
    Crate,
    Bomb,
    Explosion
};

enum class Direction {
    Up,
    Right,
    Down,
    Left
};

enum class PlayerColor {
    Unknown,
    Red,
    Green,
    Blue,
    Yellow
};

enum class EliminationReason
{
    Blast,
    Disconnected
};

enum class ActionRejectionReason
{
    WrongPhase,
    StaleRound,
    PlayerEliminated,
    BombCapacityReached,
    TileBlocked,
    MoveCooldown,
    NotLeader,
    InvalidRequest
};

enum class RoundOutcome
{
    Winner,
    Draw
};

enum class LobbyResetReason
{
    RoundFinished,
    PreparationDisconnect,
    PreparationTimeout
};

struct RoundResult
{
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;

    RoundOutcome outcome = RoundOutcome::Draw;
    std::optional<std::uint32_t> winnerId;

    // Cached when results arrive, so later disconnects do not erase it.
    std::string winnerName;
};

// Data received from the server, before model validation.
struct BoardData
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::vector<Tile> cells;
};


struct RoundRules
{
    std::uint32_t tickRateHz = 0;
    std::uint32_t bombCapacity = 0;
    std::uint32_t bombFuseTicks = 0;
    std::uint32_t blastDurationTicks = 0;
    std::uint32_t blastRange = 0;
    std::uint32_t movementIntervalTicks = 0;

    bool isValid() const
    {
        return tickRateHz > 0 &&
               bombCapacity > 0 &&
               bombFuseTicks > 0 &&
               blastDurationTicks > 0 &&
               blastRange > 0 &&
               movementIntervalTicks > 0;
    }
};