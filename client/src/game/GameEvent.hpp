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
    bool is_ready;
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

struct MatchPreparationStarted
{
    std::uint64_t roundId = 0;
};

struct PlayerSpawn
{
    std::uint32_t id;
    int x;
    int y;
};

struct MatchSetupReceived
{
    std::uint64_t roundId = 0;
    BoardData board;
    RoundRules rules;
    std::vector<PlayerSpawn> spawns;
};

struct MatchStarted
{
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
};

struct WorldSnapshotReceived
{
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
    BoardData board;
};

struct PlayerEliminatedReceived
{
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
    std::uint32_t playerId = 0;
    EliminationReason reason = EliminationReason::Blast;
};

struct ActionRejectedReceived
{
    std::uint64_t roundId = 0;
    std::uint64_t requestId = 0;
    ActionRejectionReason reason =
        ActionRejectionReason::InvalidRequest;
};

struct MatchEndedReceived
{
    RoundResult result;
};

struct LobbyResetReceived
{
    std::uint64_t roundId = 0;
    LobbyResetReason reason = LobbyResetReason::RoundFinished;
};

using GameEvent = std::variant<
    LobbyJoined,
    LobbyLeaderChanged,
    PlayerJoined,
    PlayerLeft,
    PlayerReadyChanged,
    PlayerMoved,
    MatchPreparationStarted,
    MatchSetupReceived,
    MatchStarted,
    WorldSnapshotReceived,
    PlayerEliminatedReceived,
    ActionRejectedReceived,
    MatchEndedReceived,
    LobbyResetReceived
>;
