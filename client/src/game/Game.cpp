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

void Game::apply(const PlayerSpawn& event)
{
    if (!map.contains(event.x, event.y))
    {
        return;
    }

    ensurePlayer(event.id).setPosition(event.x, event.y);
}

void Game::apply(const PlayerMoved& event)
{
    if (roundId == 0 || event.roundId != roundId ||
        event.serverTick < serverTick || !map.contains(event.x, event.y))
        return;

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
    const auto& rules = setup.rules;
    if (!joinedLobby || setup.roundId == 0 || setup.roundId == roundId ||
        !Map::validDimensions(setup.width, setup.height) ||
        setup.terrain.size() != static_cast<std::uint64_t>(setup.width) * setup.height ||
        rules.tickRateHz == 0 || rules.bombCapacity == 0 ||
        rules.bombFuseTicks == 0 || rules.blastDurationTicks == 0 ||
        rules.blastRange == 0 || rules.movementIntervalTicks == 0 ||
        setup.spawns.size() != players.size())
        return false;

    for (const auto tile : setup.terrain)
    {
        if (tile != Map::Tile::Empty && tile != Map::Tile::Wall &&
            tile != Map::Tile::Crate)
            return false;
    }

    std::unordered_set<std::uint32_t> ids;
    std::unordered_set<std::uint64_t> occupiedCells;
    for (const auto& spawn : setup.spawns)
    {
        if (spawn.x < 0 || spawn.y < 0 ||
            static_cast<std::uint32_t>(spawn.x) >= setup.width ||
            static_cast<std::uint32_t>(spawn.y) >= setup.height ||
            players.find(spawn.id) == players.end() ||
            !ids.insert(spawn.id).second)
            return false;

        const auto index = static_cast<std::uint64_t>(spawn.y) * setup.width + spawn.x;
        if (setup.terrain[index] != Map::Tile::Empty ||
            !occupiedCells.insert(index).second)
            return false;
    }
    if (ids.find(localPlayerId) == ids.end())
        return false;

    // All allocations and preparation happen before committing any changes.
    Map nextMap{setup.width, setup.height, setup.terrain};
    auto nextPlayers = players;
    for (auto& [id, player] : nextPlayers)
        player.resetRoundState();
    for (const auto& spawn : setup.spawns)
        nextPlayers.at(spawn.id).setPosition(spawn.x, spawn.y);

    using std::swap;
    swap(map, nextMap);
    players.swap(nextPlayers);
    roundId = setup.roundId;
    serverTick = 0;
    roundRules = setup.rules;
    bombs.clear();
    blasts.clear();
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

std::uint64_t Game::getRoundId() const { return roundId; }
std::uint64_t Game::getServerTick() const { return serverTick; }
const RoundRules& Game::getRoundRules() const { return roundRules; }
const std::unordered_map<std::uint64_t, Bomb>& Game::getBombs() const { return bombs; }
const std::vector<Blast>& Game::getBlasts() const { return blasts; }

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
