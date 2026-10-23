#pragma once

#include <cstdint>
#include <vector>

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

struct RoundRules
{
    std::uint32_t tickRateHz = 0;
    std::uint32_t bombCapacity = 0;
    std::uint32_t bombFuseTicks = 0;
    std::uint32_t blastDurationTicks = 0;
    std::uint32_t blastRange = 0;
    std::uint32_t movementIntervalTicks = 0;
};

struct Cell
{
    int x = 0;
    int y = 0;
};

// Storage for later authoritative bomb/explosion events; no local simulation.
struct Bomb
{
    std::uint64_t id = 0;
    std::uint32_t ownerId = 0;
    Cell position;
    std::uint32_t blastRange = 0;
    std::uint64_t explodeAtTick = 0;
};

struct Blast
{
    std::uint64_t bombId = 0;
    std::vector<Cell> cells;
    std::uint64_t expiresAtTick = 0;
};
