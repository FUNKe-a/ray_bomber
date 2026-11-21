#pragma once

#include "GameSession.hpp"

class GameSession::SessionState
{
public:
    virtual ~SessionState() = default;
    virtual State id() const = 0;

    // Called after the transport has been polled.
    // Return true when the session can process received events.
    virtual bool beginFrame(GameSession& session) const
    {
        if (!session.client.isConnected())
        {
            session.fail("Server closed the connection.");
            return false;
        }

        return true;
    }

    virtual void checkTimeout(GameSession&) const {}

    virtual void setReady(GameSession&, bool) const {}
    virtual void requestGameStart(GameSession&) const {}
    virtual void move(GameSession&, Direction) const {}
    virtual void placeBomb(GameSession&) const {}
    virtual void requestReturnToLobby(GameSession&) const {}
};

class GameSession::DisconnectedState final : public SessionState
{
public:
    State id() const override { return State::Disconnected; }
    bool beginFrame(GameSession&) const override { return false; }
};

class GameSession::FailedState final : public SessionState
{
public:
    State id() const override { return State::Failed; }
    bool beginFrame(GameSession&) const override { return false; }
};

class GameSession::ConnectingState final : public SessionState
{
public:
    State id() const override { return State::Connecting; }
    bool beginFrame(GameSession& session) const override;
};

class GameSession::JoiningState final : public SessionState
{
public:
    State id() const override { return State::Joining; }

    void checkTimeout(GameSession& session) const override
    {
        if (Clock::now() - session.stateEnteredAt >= JoinTimeout)
            session.fail("Timed out waiting for the server to accept the lobby join.");
    }
};

class GameSession::JoinedState final : public SessionState
{
public:
    State id() const override { return State::Joined; }

    void setReady(GameSession& session, bool ready) const override
    {
        session.client.setReady(ready);
    }

    void requestGameStart(GameSession& session) const override
    {
        if (session.game.isLocalPlayerLeader())
            session.client.requestGameStart();
    }
};

class GameSession::PreparingMatchState final : public SessionState
{
public:
    State id() const override { return State::PreparingMatch; }

    void checkTimeout(GameSession& session) const override
    {
        if (Clock::now() - session.stateEnteredAt >= PreparationTimeout)
            session.fail("Timed out waiting for the server's match setup.");
    }
};

class GameSession::WaitingForMatchStartState final : public SessionState
{
public:
    State id() const override { return State::WaitingForMatchStart; }

    void checkTimeout(GameSession& session) const override
    {
        if (Clock::now() - session.stateEnteredAt >= MatchStartTimeout)
            session.fail("Timed out waiting for the server to start the match.");
    }
};

class GameSession::PlayingState final : public SessionState
{
public:
    State id() const override { return State::Playing; }

    void move(GameSession& session, Direction direction) const override
    {
        session.sendMove(direction);
    }

    void placeBomb(GameSession& session) const override
    {
        session.sendPlaceBomb();
    }
};

class GameSession::FinishedState final : public SessionState
{
public:
    State id() const override { return State::Finished; }

    void requestReturnToLobby(GameSession& session) const override
    {
        session.sendReturnToLobby();
    }

    void checkTimeout(GameSession& session) const override
    {
        if (session.pendingReturnRequest &&
            Clock::now() - session.returnRequestedAt >= LobbyResetTimeout)
        {
            session.fail("Timed out waiting for the server to return to the lobby.");
        }
    }
};

template<class NextState>
void GameSession::enter()
{
    static const NextState next;
    currentState = &next;
    stateEnteredAt = Clock::now();
}

inline bool GameSession::ConnectingState::beginFrame(GameSession& session) const
{
    if (!session.client.isConnected())
    {
        if (Clock::now() - session.stateEnteredAt >= ConnectTimeout)
            session.fail("Connection timed out while connecting.");

        return false;
    }

    session.client.startReceiving();
    session.client.joinLobby(session.localName);

    if (session.client.getConnectionState() == Client::ConnectionState::Failed)
    {
        session.fail(session.client.getError());
        return false;
    }

    session.enter<JoiningState>();
    return true;
}
