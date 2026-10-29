#include "Game.hpp"
#include <utility>
#include <unordered_set>

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
    if (roundId == 0 ||
        event.roundId != roundId ||
        event.serverTick < serverTick)
    {
        return;
    }

    if (!map.contains(event.x, event.y))
    {
        return;
    }

    const auto it = players.find(event.id);
    if (it == players.end() || !it->second.hasPosition() ||
        it->second.isEliminated())
        return;

    it->second.setPosition(event.x, event.y);
    serverTick = event.serverTick;
}

void Game::apply(const LobbyLeaderChanged& event)
{
    leaderId = event.id;
}

void Game::apply(const MatchPreparationStarted&)
{
    // GameSession handles the preparation phase.
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
    serverTick = 0;
    roundRules = setup.rules;

    return true;
}

void Game::apply(const MatchSetupReceived& event)
{
    prepareRound(event);
}

void Game::apply(const MatchStarted& event)
{
    if (roundId != 0 && event.roundId == roundId)
        serverTick = event.serverTick;
}

void Game::apply(const WorldSnapshotReceived&)
{
}

void Game::apply(const PlayerEliminatedReceived&)
{
}

void Game::apply(const ActionRejectedReceived&)
{
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
