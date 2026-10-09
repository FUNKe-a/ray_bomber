#include "Game.hpp"
#include <iostream>
#include <utility>

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
    if (event.x < 0 || event.x >= Map::Width ||
        event.y < 0 || event.y >= Map::Height)
    {
        return;
    }

    ensurePlayer(event.id).setPosition(event.x, event.y);
}

void Game::apply(const PlayerMoved& event)
{
    if (event.x < 0 || event.x >= Map::Width ||
        event.y < 0 || event.y >= Map::Height)
    {
        return;
    }

    ensurePlayer(event.id).setPosition(event.x, event.y);
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
    for (const auto& spawn : event.spawns)
    {
        apply(spawn);
    }
}

void Game::apply(const MatchStarted&)
{
    // GameSession handles the transition to Playing.
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
