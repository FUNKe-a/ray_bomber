#include "Game.hpp"
#include <iostream>
#include <utility>

Game::Game(std::string localName) : localName(std::move(localName)) {}

Player& Game::ensurePlayer(std::uint32_t id)
{
    return players.try_emplace(id, id).first->second;
}

void Game::handleMessage(const Envelope& message)
{
    switch (message.payload_case())
    {
        case Envelope::kJoinLobbyResponse:
            handleJoinResponse(message.join_lobby_response());
            break;
        case Envelope::kPlayerEvent:
            handlePlayerEvent(message.player_event());
            break;
        case Envelope::kPlayerMovement:
            handleMovement(message.player_movement());
            break;
        default:
            std::cerr << "Unsupported game envelope.\n";
            break;
    }
}

void Game::handleJoinResponse(const player::JoinLobbyResponse& response)
{
    localPlayerId = response.id();
    joinedLobby = true;
    auto& participant = ensurePlayer(localPlayerId);
    // Do not replace a name already supplied by a server join event.
    if (participant.getName().empty()) participant.setName(localName);
    participant.setColor(response.color());
    std::cout << "Received JoinLobbyResponse as player " << localName << " with id " << localPlayerId << '\n';
}

void Game::handlePlayerEvent(const player::PlayerEvent& event)
{
    switch (event.event_type_case())
    {
        case player::PlayerEvent::kJoined:
        {
            auto& participant = ensurePlayer(event.id());
            participant.setName(event.joined().username());
            participant.setColor(event.joined().color());
            std::cout << "Received PlayerEvent kJoined as player " << participant.getName() << " and id " << participant.getId() << '\n';
            break;
        }
        case player::PlayerEvent::kSpawned:
        {
            auto& participant = ensurePlayer(event.id());
            participant.setPosition(
                event.spawned().x(),
                event.spawned().y()
            );
            break;
        }
        case player::PlayerEvent::kReady:
            ensurePlayer(event.id()).setReady(event.ready().is_ready());
            std::cout << "Received PlayerEvent kReady with id " << event.id() << '\n';
            break;
        case player::PlayerEvent::kLeft:
            players.erase(event.id());
            if (joinedLobby && event.id() == localPlayerId) joinedLobby = false;
            std::cout << "Player " << event.id() << " left: "
                      << event.left().reason() << '\n';
            break;
        default:
            std::cerr << "Player event has no supported event type.\n";
            break;
    }
}

void Game::handleMovement(const player::PlayerMovement& movement)
{
    // The current demo map is local; require an in-bounds tile for safe drawing.
    // Do not validate server authority against locally generated wall positions.
    if (movement.x() < 0 || movement.x() >= Map::Width ||
        movement.y() < 0 || movement.y() >= Map::Height)
    {
        std::cerr << "Ignoring movement outside the demo map.\n";
        return;
    }
    ensurePlayer(movement.id()).setPosition(movement.x(), movement.y());
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
