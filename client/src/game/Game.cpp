#include "Game.hpp"
#include <utility>
#include <unordered_set>
#include <algorithm>

Game::Game(std::string localName) : localName(std::move(localName)) {}

Player& Game::ensurePlayer(std::uint32_t id)
{
    return players.try_emplace(id, id).first->second;
}

void Game::handleEvent(const GameEvent& event)
{
    std::visit([this](const auto& value) {
            apply(value);
        },
        event
    );
}

bool Game::prepareRound(const MatchSetupReceived& setup)
{
    if (!joinedLobby ||
        setup.roundId == 0 ||
        setup.roundId == roundId ||
        !setup.rules.isValid())
    {
        return false;
    }

    auto nextMap = Map::fromBoard(setup.board);

    if (!nextMap)
    {
        return false;
    }

    // Initial setup must not contain active bombs or explosions.
    for (const auto tile : nextMap->getCells())
    {
        if (tile == Tile::Bomb || tile == Tile::Explosion)
        {
            return false;
        }
    }

    // Currently, every lobby participant takes part in the round.
    if (setup.spawns.size() != players.size())
    {
        return false;
    }

    std::unordered_set<std::uint32_t> spawnedPlayers;
    std::unordered_set<std::size_t> occupiedCells;

    for (const auto& spawn : setup.spawns)
    {
        if (players.find(spawn.id) == players.end() ||
            !spawnedPlayers.insert(spawn.id).second)
        {
            return false;
        }

        if (!nextMap->contains(spawn.x, spawn.y) ||
            nextMap->getTile(spawn.x, spawn.y) != Tile::Floor)
        {
            return false;
        }

        const auto index =
            static_cast<std::size_t>(spawn.y) *
            nextMap->getWidth() + spawn.x;

        if (!occupiedCells.insert(index).second)
        {
            return false;
        }
    }

    if (spawnedPlayers.find(localPlayerId) == spawnedPlayers.end())
    {
        return false;
    }

    // Prepare player state without changing the current model.
    auto nextPlayers = players;

    for (auto& entry : nextPlayers)
    {
        entry.second.resetRoundState();
    }

    for (const auto& spawn : setup.spawns)
    {
        nextPlayers.at(spawn.id).setPosition(spawn.x, spawn.y);
    }

    // Commit only after validation and preparation have succeeded.
    map.swap(*nextMap);
    players.swap(nextPlayers);

    roundId = setup.roundId;
    roundRules = setup.rules;

    serverTick = 0;
    roundStartTick = 0;

    lastBoardTick.reset();
    lastMovementTicks.clear();
    lastActionRejection.reset();

    return true;
}


void Game::apply(const LobbyJoined& event)
{
    localPlayerId = event.id;
	leaderId = event.leader_id;
    joinedLobby = true;

    auto& participant = ensurePlayer(event.id);

    if (participant.getName().empty())
        participant.setName(localName);

    participant.setColor(event.color);

	for (const auto& player : event.players) {
		auto& other = ensurePlayer(player.id);
		other.setName(player.username);
		other.setColor(player.color);
	}
}

void Game::apply(const PlayerJoined& event)
{
    auto& participant = ensurePlayer(event.id);
    participant.setName(event.name);
    participant.setColor(event.color);
}

void Game::apply(const PlayerLeft& event)
{
    players.erase(event.id);

    if (joinedLobby && event.id == localPlayerId)
        joinedLobby = false;

    if (leaderId && event.id == *leaderId)
    {
        leaderId.reset();
    }
}

void Game::apply(const PlayerReadyChanged& event)
{
    ensurePlayer(event.id).setReady(event.ready);
}

void Game::apply(const PlayerMoved& event)
{
    if (roundId == 0 || event.roundId != roundId || event.serverTick < roundStartTick)
        return;

    const auto playerIt = players.find(event.id);

    if (playerIt == players.end() || !playerIt->second.hasPosition() || playerIt->second.isEliminated())
        return;

    if (!map.contains(event.x, event.y))
        return;

    const auto tickIt = lastMovementTicks.find(event.id);

    if (tickIt != lastMovementTicks.end() && event.serverTick < tickIt->second)
        return;

    // Įrašome tick prieš keičiant poziciją:
    // naujo įrašo sukūrimas gali pareikalauti atminties.
    lastMovementTicks.insert_or_assign(event.id, event.serverTick);
    playerIt->second.setPosition(event.x, event.y);
    serverTick = std::max(serverTick, event.serverTick);
}

void Game::apply(const LobbyLeaderChanged& event)
{
    leaderId = event.id;
}

void Game::apply(const MatchPreparationStarted&)
{
    // GameSession handles the preparation phase.
}

void Game::apply(const MatchSetupReceived& event)
{
    prepareRound(event);
}

void Game::apply(const MatchStarted& event)
{
    if (roundId == 0 || event.roundId != roundId)
        return;

    roundStartTick = event.serverTick;
    serverTick = std::max(serverTick, event.serverTick);
}

void Game::apply(const WorldSnapshotReceived& event)
{
    if (roundId == 0 || event.roundId != roundId)
        return;

    if (event.serverTick < roundStartTick)
        return;

    // Vienam tick serveris siunčia daugiausia vieną snapshot.
    if (lastBoardTick && event.serverTick <= *lastBoardTick)
        return;

    // Raundo metu arenos matmenys nesikeičia.
    if (event.board.width != static_cast<std::uint32_t>(map.getWidth()) ||
        event.board.height != static_cast<std::uint32_t>(map.getHeight()))
        return;

    auto nextMap = Map::fromBoard(event.board);
    if (!nextMap)
        return;

    map.swap(*nextMap);

    lastBoardTick = event.serverTick;
    serverTick = std::max(serverTick, event.serverTick);
}

void Game::apply(const PlayerEliminatedReceived& event)
{
    if (roundId == 0 || event.roundId != roundId || event.serverTick < roundStartTick)
        return;

    const auto it = players.find(event.playerId);

    if (it == players.end() || it->second.isEliminated())
        return;

    it->second.eliminate();

    serverTick = std::max(serverTick, event.serverTick);
}

void Game::apply(const ActionRejectedReceived& event)
{
    if (roundId == 0 || event.roundId != roundId || event.requestId == 0)
        return;

    lastActionRejection = event;
}

std::uint64_t Game::getRoundId() const 
{ 
    return roundId; 
}

std::uint64_t Game::getServerTick() const 
{ 
    return serverTick; 
}

const RoundRules& Game::getRoundRules() const 
{ 
    return roundRules; 
}

const Map& Game::getMap() const 
{ 
    return map; 
}

const std::unordered_map<std::uint32_t, Player>& Game::getPlayers() const 
{
    return players; 
}

const Player* Game::getPlayer(std::uint32_t id) const
{
    const auto it = players.find(id);
    return it == players.end() ? nullptr : &it->second;
}

const Player* Game::getLocalPlayer() const
{ 
    return joinedLobby ? getPlayer(localPlayerId) : nullptr; 
}

std::uint32_t Game::getLocalPlayerId() const 
{ 
    return localPlayerId; 
}

bool Game::hasJoinedLobby() const 
{ 
    return joinedLobby;
}

std::optional<std::uint32_t> Game::getLeaderId() const
{
    return leaderId;
}

bool Game::isLocalPlayerLeader() const
{
    return hasJoinedLobby() &&
           leaderId.has_value() &&
           *leaderId == getLocalPlayerId();
}

std::optional<std::uint64_t> Game::getLastBoardTick() const
{
    return lastBoardTick;
}

const std::optional<ActionRejectedReceived>& Game::getLastActionRejection() const
{
    return lastActionRejection;
}