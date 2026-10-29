#pragma once

#include "Map.hpp"
#include "Player.hpp"
#include "GameEvent.hpp"
#include "EventSignal.hpp"
#include "GameNotification.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <optional>

class Game
{
public:
    explicit Game(std::string localName = "Player");
    void handleEvent(const GameEvent& event);
    bool prepareRound(const MatchSetupReceived& setup);
    std::uint64_t getRoundId() const;
    std::uint64_t getServerTick() const;
    const RoundRules& getRoundRules() const;
    const Map& getMap() const;
    const std::unordered_map<std::uint32_t, Player>& getPlayers() const;
    const Player* getPlayer(std::uint32_t id) const;
    const Player* getLocalPlayer() const;
    std::uint32_t getLocalPlayerId() const;
    std::optional<std::uint64_t> getLastBoardTick() const;
    const std::optional<ActionRejectedReceived>& getLastActionRejection() const;
    bool hasJoinedLobby() const;
    std::optional<std::uint32_t> getLeaderId() const;
    bool isLocalPlayerLeader() const;
    void resetRound();
    
    using PresentationCallback = EventSignal<GameNotification>::Callback;
    using PresentationSubscription = EventSignal<GameNotification>::Subscription;
    PresentationSubscription observe(PresentationCallback callback);

private:
    Player& ensurePlayer(std::uint32_t id);
    
    void apply(const LobbyJoined& event);
    void apply(const LobbyLeaderChanged& event);
    void apply(const PlayerJoined& event);
    void apply(const PlayerLeft& event);
    void apply(const PlayerReadyChanged& event);
    void apply(const PlayerMoved& event);
    void apply(const MatchPreparationStarted& event);
    void apply(const MatchSetupReceived& event);
    void apply(const MatchStarted& event);
    void apply(const WorldSnapshotReceived& event);
    void apply(const PlayerEliminatedReceived& event);
    void apply(const ActionRejectedReceived& event);
    void apply(const MatchEndedReceived& event);
    void apply(const LobbyResetReceived& event);    

    Map map;
    std::uint64_t roundId = 0;
    std::uint64_t serverTick = 0;
    RoundRules roundRules;
    std::unordered_map<std::uint32_t, Player> players;
    std::string localName;
    std::uint32_t localPlayerId = 0;
    bool joinedLobby = false;
    std::optional<std::uint32_t> leaderId;
    std::uint64_t roundStartTick = 0;
    std::optional<std::uint64_t> lastBoardTick;
    std::unordered_map<std::uint32_t, std::uint64_t> lastMovementTicks;
    std::optional<ActionRejectedReceived> lastActionRejection;
    EventSignal<GameNotification> presentationEvents;
};
