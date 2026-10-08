#include "GameSession.hpp"
#include "Map.hpp"
#include "GameEvent.hpp"

#include <utility>
#include <unordered_set>
#include <iostream>

namespace
{
    bool isValidMatchSetup(
        const MatchSetupReceived& setup,
        const Game& game
    )
    {
        if (!game.hasJoinedLobby())
        {
            return false;
        }

        std::unordered_set<std::uint32_t> playerIds;
        bool includesLocalPlayer = false;

        for (const auto& spawn : setup.spawns)
        {
            if (spawn.x < 0 || spawn.x >= Map::Width ||
                spawn.y < 0 || spawn.y >= Map::Height)
            {
                return false;
            }

            if (!playerIds.insert(spawn.id).second)
            {
                return false;
            }

            if (spawn.id == game.getLocalPlayerId())
            {
                includesLocalPlayer = true;
            }
        }

        return includesLocalPlayer;
    }
}

void GameSession::connect(
    const std::string& address,
    std::uint16_t port,
    std::string playerName
)
{
    disconnect();

    localName = std::move(playerName);
    game = Game{localName};

    connectStartedAt = Clock::now();
    state = State::Connecting;

    client.connect(address, port);
}

void GameSession::disconnect()
{
    client.disconnect();

    game = Game{};
    localName.clear();
    errorMessage.clear();
    state = State::Disconnected;
}

void GameSession::update()
{
    client.poll();

    if (state == State::Disconnected ||
        state == State::Failed)
    {
        return;
    }

    if (client.getConnectionState() ==
        Client::ConnectionState::Failed)
    {
        fail(client.getError());
        return;
    }

    if (state == State::Connecting)
    {
        if (!client.isConnected())
        {
            if (Clock::now() - connectStartedAt >= ConnectTimeout)
            {
                fail("Connection timed out while connecting.");
            }

            return;
        }

        joinStartedAt = Clock::now();

        client.startReceiving();
        client.joinLobby(localName);

        // Sending the request can fail immediately.
        if (client.getConnectionState() ==
            Client::ConnectionState::Failed)
        {
            fail(client.getError());
            return;
        }

        state = State::Joining;
    }

    if (!client.isConnected())
    {
        fail("Server closed the connection.");
        return;
    }

    GameEvent event;

    while (client.receive(event))
    {
        if (std::holds_alternative<MatchPreparationStarted>(event))
        {
            if (state != State::Joined)
            {
                fail("Received game start outside the lobby.");
                return;
            }

            state = State::PreparingMatch;
            continue;
        }

        if (const auto* setup = std::get_if<MatchSetupReceived>(&event))
        {
            if (state != State::PreparingMatch)
            {
                fail("Received match setup outside match preparation.");
                return;
            }

            if (!isValidMatchSetup(*setup, game))
            {
                fail("Server sent an invalid match setup.");
                return;
            }

            game.handleEvent(event);

            // Current preparation consists of applying spawn positions.
            client.matchReady();

            if (client.getConnectionState() == Client::ConnectionState::Failed)
            {
                fail(client.getError());
                return;
            }

            state = State::WaitingForMatchStart;
            continue;
        }

        if (std::holds_alternative<MatchStarted>(event))
        {
            if (state != State::WaitingForMatchStart)
            {
                fail("Received match start before preparation completed.");
                return;
            }

            state = State::Playing;
            continue;
        }

        game.handleEvent(event);

        if (state == State::Joining && game.hasJoinedLobby())
        {
            state = State::Joined;
        }
        else if (state != State::Joining && !game.hasJoinedLobby())
        {
            fail("You are no longer in the game.");
            return;
        }
    }

    if (state == State::Joining &&
        Clock::now() - joinStartedAt >= JoinTimeout)
    {
        fail("Timed out waiting for the server to accept the lobby join.");
    }
}

void GameSession::setReady(bool ready)
{
    if (state == State::Joined)
    {
        client.setReady(ready);
    }
}

void GameSession::move(Direction direction)
{
    if (state == State::Playing)
    {
        client.move(direction);
    }
}

void GameSession::requestGameStart()
{
    if (state != State::Joined || !game.isLocalPlayerLeader())
    {
        return;
    }

    client.requestGameStart();
}

GameSession::State GameSession::getState() const
{
    return state;
}

const std::string& GameSession::getError() const
{
    return errorMessage;
}

const Game& GameSession::getGame() const
{
    return game;
}

void GameSession::fail(std::string message)
{
    if (message.empty())
    {
        message = "Connection failed.";
    }

    disconnect();

    errorMessage = std::move(message);
    state = State::Failed;
}