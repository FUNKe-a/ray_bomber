#pragma once

#include "Client.hpp"
#include "Game.hpp"
#include "GameTypes.hpp"

#include <chrono>
#include <cstdint>
#include <string>

class GameSession
{
public:
    enum class State {
        Disconnected,
        Connecting,
        Joining,
        Joined,
        Failed
    };

    void connect(
        const std::string& address,
        std::uint16_t port,
        std::string playerName
    );

    void disconnect();
    void update();

    void setReady(bool ready);
    void move(Direction direction);

    State getState() const;
    const std::string& getError() const;
    const Game& getGame() const;

private:
    using Clock = std::chrono::steady_clock;

    void fail(std::string message);

    Client client;
    Game game;

    State state = State::Disconnected;
    std::string localName;
    std::string errorMessage;
    Clock::time_point joinStartedAt{};
};