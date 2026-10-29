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
    roundResult.reset();
    pendingReturnRequest.reset();
    noticeMessage.clear();
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

            roundResult.reset();
            pendingReturnRequest.reset();
            noticeMessage.clear();

            preparationStartedAt = Clock::now();

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

            matchWaitStartedAt = Clock::now();
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

        if (const auto* ended = std::get_if<MatchEndedReceived>(&event))
        {
            const auto& result = ended->result;

            if (result.roundId == 0 || result.roundId != game.getRoundId())
                continue;

            // Ignore a duplicate result for the finished round.
            if (state == State::Finished)
                continue;

            if (state != State::Playing)
            {
                fail("Received round results before gameplay started.");
                return;
            }

            roundResult = result;

            if (roundResult->winnerId)
            {
                const auto winnerId = *roundResult->winnerId;
                const auto* winner = game.getPlayer(winnerId);

                roundResult->winnerName =
                    winner && !winner->getName().empty()
                        ? winner->getName()
                        : "Player " + std::to_string(winnerId);
            }

            pendingReturnRequest.reset();
            noticeMessage.clear();

            state = State::Finished;
            continue;
        }

        if (const auto* reset = std::get_if<LobbyResetReceived>(&event))
        {
            const auto expectedRound =
                preparingRoundId.value_or(game.getRoundId());

            if (expectedRound == 0 || reset->roundId != expectedRound)
                continue;

            const bool isPreparing =
                state == State::PreparingMatch ||
                state == State::WaitingForMatchStart;

            if (reset->reason == LobbyResetReason::RoundFinished)
            {
                if (state != State::Finished)
                {
                    fail("Received a finished-round reset in an invalid phase.");
                    return;
                }

                noticeMessage = "Round finished. Ready up for the next round.";
            }
            else
            {
                if (!isPreparing)
                {
                    fail("Received a preparation reset in an invalid phase.");
                    return;
                }

                noticeMessage =
                    reset->reason == LobbyResetReason::PreparationTimeout
                        ? "Preparation timed out. Ready up to try again."
                        : "Preparation cancelled because a player disconnected.";
            }

            game.resetRound();

            roundResult.reset();
            pendingReturnRequest.reset();
            preparingRoundId.reset();

            nextRequestId = 1;
            state = State::Joined;
            continue;
        }

        if (const auto* rejected = std::get_if<ActionRejectedReceived>(&event))
        {
            if (state == State::Finished &&
                rejected->roundId == game.getRoundId() &&
                pendingReturnRequest &&
                rejected->requestId == *pendingReturnRequest)
            {
                pendingReturnRequest.reset();
                noticeMessage = "Return request rejected. You can try again.";
            }
        }

        // Gameplay updates must not alter a round before StartMatch.
        const bool isWorldEvent =
            std::holds_alternative<PlayerMoved>(event) ||
            std::holds_alternative<WorldSnapshotReceived>(event) ||
            std::holds_alternative<PlayerEliminatedReceived>(event);

        if (isWorldEvent && state != State::Playing)
            continue;

        if (std::holds_alternative<ActionRejectedReceived>(event) &&
            state != State::Playing && state != State::Finished)
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

    const auto now = Clock::now();

    if (state == State::Joining &&
        now - joinStartedAt >= JoinTimeout)
    {
        fail("Timed out waiting for the server to accept the lobby join.");
        return;
    }

    if (state == State::PreparingMatch &&
        now - preparationStartedAt >= PreparationTimeout)
    {
        fail("Timed out waiting for the server's match setup.");
        return;
    }

    if (state == State::WaitingForMatchStart &&
        now - matchWaitStartedAt >= MatchStartTimeout)
    {
        fail("Timed out waiting for the server to start the match.");
        return;
    }

    if (state == State::Finished &&
        pendingReturnRequest &&
        now - returnRequestedAt >= LobbyResetTimeout)
    {
        fail("Timed out waiting for the server to return to the lobby.");
        return;
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

Game::PresentationSubscription GameSession::observe(Game::PresentationCallback callback)
{
    return game.observe(std::move(callback));
}

const std::optional<RoundResult>& GameSession::getRoundResult() const
{
    return roundResult;
}

const std::string& GameSession::getNotice() const
{
    return noticeMessage;
}

bool GameSession::isWaitingForLobbyReset() const
{
    return pendingReturnRequest.has_value();
}

void GameSession::requestReturnToLobby()
{
    if (state != State::Finished ||
        !client.isConnected() ||
        !roundResult ||
        pendingReturnRequest ||
        !game.isLocalPlayerLeader())
    {
        return;
    }

    if (nextRequestId == 0)
    {
        fail("Request IDs exhausted.");
        return;
    }

    const auto requestId = nextRequestId++;

    pendingReturnRequest = requestId;
    returnRequestedAt = Clock::now();
    noticeMessage.clear();

    client.returnToLobby(roundResult->roundId, requestId);

    if (client.getConnectionState() == Client::ConnectionState::Failed)
        fail(client.getError());
}