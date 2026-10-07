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
    joinedLobby = true;

    auto& participant = ensurePlayer(event.id);

    if (participant.getName().empty())
        participant.setName(localName);

    participant.setColor(event.color);
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
}

void Game::apply(const PlayerReadyChanged& event)
{
    ensurePlayer(event.id).setReady(event.ready);
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

const Map& Game::getMap() const { return map; }
const std::unordered_map<std::uint32_t, Player>& Game::getPlayers() const { return players; }
const Player* Game::getPlayer(std::uint32_t id) const
{
    const auto it = players.find(id);
    return it == players.end() ? nullptr : &it->second;
}
const Player* Game::getLocalPlayer() const
{ return joinedLobby ? getPlayer(localPlayerId) : nullptr; }
std::uint32_t Game::getLocalPlayerId() const { return localPlayerId; }
bool Game::hasJoinedLobby() const { return joinedLobby; }
