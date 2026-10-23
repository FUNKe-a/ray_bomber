#include "GameSession.hpp"
#include "GameEvent.hpp"

#include <utility>
#include <exception>
#include <limits>

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
    nextRequestId = 1;
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

            try
            {
                if (!game.prepareRound(*setup))
                {
                    fail("Server sent an invalid match setup.");
                    return;
                }
            }
            catch (const std::exception&)
            {
                fail("Could not prepare the round.");
                return;
            }

            nextRequestId = 1;
            client.matchReady(game.getRoundId());

            if (client.getConnectionState() == Client::ConnectionState::Failed)
            {
                fail(client.getError());
                return;
            }

            state = State::WaitingForMatchStart;
            continue;
        }

        if (const auto* started = std::get_if<MatchStarted>(&event))
        {
            if (state != State::WaitingForMatchStart)
            {
                fail("Received match start before preparation completed.");
                return;
            }

            if (started->roundId != game.getRoundId())
            {
                fail("Received match start for a different round.");
                return;
            }

            game.handleEvent(event);
            state = State::Playing;
            continue;
        }

        // Gameplay updates must not alter a round before StartMatch.
        if (std::holds_alternative<PlayerMoved>(event) && state != State::Playing)
            continue;

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
        if (nextRequestId == std::numeric_limits<std::uint64_t>::max())
        {
            fail("Gameplay request IDs exhausted.");
            return;
        }
        client.move(direction, game.getRoundId(), nextRequestId++);
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
