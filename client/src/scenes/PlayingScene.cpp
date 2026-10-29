#include "PlayingScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"

#include <raylib.h>

#include <string>
#include <utility>

namespace
{
    const char* rejectionText(ActionRejectionReason reason)
    {
        switch (reason)
        {
            case ActionRejectionReason::WrongPhase:
                return "Gameplay actions are unavailable.";

            case ActionRejectionReason::StaleRound:
                return "The action belongs to another round.";

            case ActionRejectionReason::PlayerEliminated:
                return "You are eliminated.";

            case ActionRejectionReason::BombCapacityReached:
                return "Bomb capacity reached.";

            case ActionRejectionReason::TileBlocked:
                return "The tile is blocked.";

            case ActionRejectionReason::MoveCooldown:
                return "Movement is on cooldown.";

            case ActionRejectionReason::NotLeader:
                return "Only the leader can perform this action.";

            case ActionRejectionReason::InvalidRequest:
                return "Invalid action.";
        }

        return "Action rejected.";
    }
}

PlayingScene::PlayingScene(GameSession& session)
    : session(session),
      presentationSubscription(
          session.observe(
              [this](const GameNotification& notification) {
                  onGameNotification(notification);
              }
          )
      )
{
}

std::optional<SceneTransition> PlayingScene::update()
{
    if (IsKeyPressed(KEY_ESCAPE))
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, {}};
    }

    const auto state = session.getState();

    if (state == GameSession::State::Failed)
    {
        std::string error = session.getError();

        if (error.empty())
        {
            error = "Connection lost.";
        }

        session.disconnect();

        return SceneTransition{SceneType::Menu, std::move(error)};
    }

    if (state == GameSession::State::Disconnected)
    {
        return SceneTransition{
            SceneType::Menu,
            "Disconnected from server."
        };
    }


    if (state == GameSession::State::Finished)
    {
        return SceneTransition{SceneType::Results, {}};
    }

    if (state == GameSession::State::Joined ||
        state == GameSession::State::PreparingMatch ||
        state == GameSession::State::WaitingForMatchStart)
    {
        return SceneTransition{SceneType::Lobby, {}};
    }

    if (state != GameSession::State::Playing)
    {
        return std::nullopt;
    }

    input.handleInput(session);

    return std::nullopt;
}

void PlayingScene::render() const
{
    renderer.render(session.getGame());

    if (!actionMessage.empty() && GetTime() < actionMessageExpiresAt)
    {
        DrawText(
            actionMessage.c_str(),
            20,
            GetScreenHeight() - 30,
            20,
            ORANGE
        );
    }
}

void PlayingScene::onGameNotification(const GameNotification& notification)
{
    if (const auto* snapshot = std::get_if<WorldSnapshotApplied>(&notification))
    {
        TraceLog(LOG_DEBUG, "WORLD: snapshot applied, tick=%llu",
            static_cast<unsigned long long>(snapshot->serverTick)
        );

        return;
    }

    if (const auto* elimination = std::get_if<PlayerEliminationApplied>(&notification))
    {
        TraceLog( LOG_INFO, "WORLD: player eliminated, id=%u",
            static_cast<unsigned int>(elimination->playerId)
        );

        if (elimination->playerId == session.getGame().getLocalPlayerId())
        {
            actionMessage = "You are eliminated.";
            actionMessageExpiresAt = GetTime() + 3.0;
        }

        return;
    }

    if (const auto* rejection = std::get_if<GameplayActionRejected>(&notification))
    {
        actionMessage = rejectionText(rejection->reason);
        actionMessageExpiresAt = GetTime() + 2.0;

        TraceLog( LOG_INFO, "ACTION: request=%llu rejected: %s",
            static_cast<unsigned long long>(rejection->requestId),
            actionMessage.c_str()
        );
    }
}