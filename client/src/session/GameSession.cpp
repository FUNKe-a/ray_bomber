#include "GameSession.hpp"
#include "GameSessionStates.hpp"
#include "GameEvent.hpp"

#include <utility>
#include <exception>

GameSession::GameSession()
{
    enter<DisconnectedState>();
}

GameSession::State GameSession::getState() const
{
    return currentState->id();
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

    enter<ConnectingState>();

    client.connect(address, port);
}

void GameSession::disconnect()
{
    client.disconnect();

    game = Game{};
    localName.clear();
    errorMessage.clear();
    enter<DisconnectedState>();
    nextRequestId = 1;
    preparingRoundId.reset();
    roundResult.reset();
    pendingReturnRequest.reset();
    noticeMessage.clear();
}

void GameSession::update()
{
    client.poll();

    if (getState() == State::Disconnected || getState() == State::Failed)
        return;

    if (client.getConnectionState() == Client::ConnectionState::Failed)
    {
        fail(client.getError());
        return;
    }

    if(!currentState->beginFrame(*this))
        return;

    GameEvent event;

    while (client.receive(event))
    {
        if (const auto* preparation = std::get_if<MatchPreparationStarted>(&event))
        {
            if (getState() != State::Joined || preparation->roundId == 0)
            {
                fail("Received invalid match preparation.");
                return;
            }

            roundResult.reset();
            pendingReturnRequest.reset();
            noticeMessage.clear();

            preparingRoundId = preparation->roundId;
            enter<PreparingMatchState>();
            continue;
        }

        if (const auto* setup = std::get_if<MatchSetupReceived>(&event))
        {
            if (getState() != State::PreparingMatch || !preparingRoundId || setup->roundId != *preparingRoundId)
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

            enter<WaitingForMatchStartState>();
            continue;
        }

        if (const auto* started = std::get_if<MatchStarted>(&event))
        {
            if (getState() != State::WaitingForMatchStart ||
                !preparingRoundId ||
                started->roundId != *preparingRoundId ||
                started->roundId != game.getRoundId())
            {
                fail("Received match start for an unprepared round.");
                return;
            }

            game.handleEvent(event);

            preparingRoundId.reset();
            enter<PlayingState>();
            continue;
        }

        if (const auto* ended = std::get_if<MatchEndedReceived>(&event))
        {
            const auto& result = ended->result;

            if (result.roundId == 0 || result.roundId != game.getRoundId())
                continue;

            // Ignore a duplicate result for the finished round.
            if (getState() == State::Finished)
                continue;

            if (getState() != State::Playing)
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

            enter<FinishedState>();
            continue;
        }

        if (const auto* reset = std::get_if<LobbyResetReceived>(&event))
        {
            const auto expectedRound =
                preparingRoundId.value_or(game.getRoundId());

            if (expectedRound == 0 || reset->roundId != expectedRound)
                continue;

            const bool isPreparing =
                getState() == State::PreparingMatch ||
                getState() == State::WaitingForMatchStart;

            if (reset->reason == LobbyResetReason::RoundFinished)
            {
                if (getState() != State::Finished)
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
            enter<JoinedState>();
            continue;
        }

        if (const auto* rejected = std::get_if<ActionRejectedReceived>(&event))
        {
            if (getState() == State::Finished &&
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

        if (isWorldEvent && getState() != State::Playing)
            continue;

        if (std::holds_alternative<ActionRejectedReceived>(event) &&
            getState() != State::Playing && getState() != State::Finished)
            continue;

        game.handleEvent(event);

        if (getState() == State::Joining && game.hasJoinedLobby())
        {
            enter<JoinedState>();
        }
        else if (getState() != State::Joining && !game.hasJoinedLobby())
        {
            fail("You are no longer in the game.");
            return;
        }
    }

    currentState->checkTimeout(*this);
}

void GameSession::setReady(bool ready)
{
    currentState->setReady(*this, ready);
}

void GameSession::move(Direction direction)
{
    currentState->move(*this, direction);
}

void GameSession::placeBomb()
{
    currentState->placeBomb(*this);
}

void GameSession::requestGameStart()
{
    currentState->requestGameStart(*this);
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
    enter<FailedState>();
}

std::optional<std::uint64_t> GameSession::takeGameplayRequestId()
{
    if (!client.isConnected())
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
    currentState->requestReturnToLobby(*this);
}

void GameSession::sendMove(Direction direction)
{
    const auto requestId = takeGameplayRequestId();
    if (!requestId)
        return;

    client.move(direction, game.getRoundId(), *requestId);
}

void GameSession::sendPlaceBomb()
{
    const auto requestId = takeGameplayRequestId();
    if (!requestId)
        return;

    client.placeBomb(game.getRoundId(), *requestId);
}

void GameSession::sendReturnToLobby()
{
    if (!client.isConnected() ||
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