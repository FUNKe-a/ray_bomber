#pragma once

#include "Client.hpp"
#include "Game.hpp"
#include "GameTypes.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <optional>

class GameSession
{
public:
    enum class State {
        Disconnected,
        Connecting,
        Joining,
        Joined,
        PreparingMatch,
        WaitingForMatchStart,
        Playing,
        Finished,
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
    void placeBomb();
    void requestGameStart();

    State getState() const;
    const std::string& getError() const;
    const Game& getGame() const;

    Game::PresentationSubscription observe(Game::PresentationCallback callback);

    const std::optional<RoundResult>& getRoundResult() const;
    const std::string& getNotice() const;

    bool isWaitingForLobbyReset() const;
    void requestReturnToLobby();

private:
    using Clock = std::chrono::steady_clock;

    void fail(std::string message);
    std::optional<std::uint64_t> takeGameplayRequestId();

    Client client;
    Game game;

    State state = State::Disconnected;
    std::string localName;
    std::string errorMessage;
    std::optional<std::uint64_t> preparingRoundId;
    std::uint64_t nextRequestId = 1;
    
    static constexpr std::chrono::seconds ConnectTimeout{10};
    static constexpr std::chrono::seconds JoinTimeout{10};

    Clock::time_point connectStartedAt{};
    Clock::time_point joinStartedAt{};

    std::optional<RoundResult> roundResult;
    std::optional<std::uint64_t> pendingReturnRequest;

    std::string noticeMessage;

    Clock::time_point preparationStartedAt{};
    Clock::time_point matchWaitStartedAt{};
    Clock::time_point returnRequestedAt{};

    static constexpr std::chrono::seconds PreparationTimeout{20};
    static constexpr std::chrono::seconds MatchStartTimeout{20};
    static constexpr std::chrono::seconds LobbyResetTimeout{10};
};