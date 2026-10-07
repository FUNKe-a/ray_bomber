#include "GameSession.hpp"

#include "GameEvent.hpp"

#include <utility>

void GameSession::connect(
    const std::string& address,
    std::uint16_t port,
    std::string playerName
)
{
    disconnect();

    localName = std::move(playerName);
    game = Game{localName};

    client.connect(address, port);
    state = State::Connecting;
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
            return;

        client.startReceiving();
        client.joinLobby(localName);

        joinStartedAt = Clock::now();
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
        game.handleEvent(event);

        // Game clears this flag when the local player leaves.
        if (state == State::Joined && !game.hasJoinedLobby())
        {
            fail("You are no longer in the lobby.");
            return;
        }

        if (state == State::Joining && game.hasJoinedLobby())
        {
            state = State::Joined;
        }
    }

    if (state == State::Joining &&
        Clock::now() - joinStartedAt >= std::chrono::seconds{10})
    {
        fail("No lobby response received within 10 seconds.");
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
    if (state == State::Joined)
    {
        client.move(direction);
    }
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