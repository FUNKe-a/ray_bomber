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
    preparingRoundId.reset();
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
        if (const auto* preparation = std::get_if<MatchPreparationStarted>(&event))
        {
            if (state != State::Joined || preparation->roundId == 0)
            {
                fail("Received invalid match preparation.");
                return;
            }

            preparingRoundId = preparation->roundId;
            state = State::PreparingMatch;
            continue;
        }

        if (const auto* setup = std::get_if<MatchSetupReceived>(&event))
        {
            if (state != State::PreparingMatch || !preparingRoundId || setup->roundId != *preparingRoundId)
            {
                fail("Received setup for an unexpected round.");
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
            if (state != State::WaitingForMatchStart ||
                !preparingRoundId ||
                started->roundId != *preparingRoundId ||
                started->roundId != game.getRoundId())
            {
                fail("Received match start for an unprepared round.");
                return;
            }

            if (std::holds_alternative<PlayerMoved>(event) &&
                state != State::Playing)
            {
                continue;
            }

            game.handleEvent(event);

            preparingRoundId.reset();
            state = State::Playing;
            continue;
        }

        // Gameplay updates must not alter a round before StartMatch.
        const bool isGameplayEvent =
            std::holds_alternative<PlayerMoved>(event) ||
            std::holds_alternative<WorldSnapshotReceived>(event) ||
            std::holds_alternative<PlayerEliminatedReceived>(event) ||
            std::holds_alternative<ActionRejectedReceived>(event);

        if (isGameplayEvent && state != State::Playing)
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
    const auto requestId = takeGameplayRequestId();
    if (!requestId)
        return;

    client.move(direction, game.getRoundId(), *requestId);
}

void GameSession::placeBomb()
{
    const auto requestId = takeGameplayRequestId();
    if (!requestId)
        return;

    client.placeBomb(game.getRoundId(), *requestId);
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

std::optional<std::uint64_t> GameSession::takeGameplayRequestId()
{
    if (state != State::Playing || !client.isConnected())
        return std::nullopt;

    const auto* player = game.getLocalPlayer();

    if (player == nullptr || !player->hasPosition() || player->isEliminated())
        return std::nullopt;

    if (nextRequestId == 0)
    {
        fail("Gameplay request IDs exhausted.");
        return std::nullopt;
    }

    return nextRequestId++;
}