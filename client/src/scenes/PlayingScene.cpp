#include "PlayingScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <algorithm>
#include <vector>
#include <string>
#include <utility>

namespace
{
    constexpr float ScreenMargin = 16.0f;
    constexpr float PanelGap = 16.0f;
    constexpr float HudWidth = 272.0f;

    Rectangle hudRectangle()
    {
        return {
            static_cast<float>(GetScreenWidth()) -
                ScreenMargin - HudWidth,
            ScreenMargin,
            HudWidth,
            static_cast<float>(GetScreenHeight()) -
                2.0f * ScreenMargin
        };
    }

    Rectangle arenaRectangle()
    {
        const auto hud = hudRectangle();

        return {
            ScreenMargin,
            ScreenMargin,
            hud.x - ScreenMargin - PanelGap,
            static_cast<float>(GetScreenHeight()) -
                2.0f * ScreenMargin
        };
    }

    Color playerColor(PlayerColor color)
    {
        switch (color)
        {
            case PlayerColor::Red:    return RED;
            case PlayerColor::Green:  return GREEN;
            case PlayerColor::Blue:   return BLUE;
            case PlayerColor::Yellow: return YELLOW;
            case PlayerColor::Unknown:
                return GRAY;
        }

        return GRAY;
    }
    
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
    const Rectangle arena = arenaRectangle();

    DrawRectangleRec(arena, Color{225, 225, 225, 255});

    renderer.render(session.getGame(), arena);
    renderHud(hudRectangle());
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

void PlayingScene::renderHud(Rectangle bounds) const
{
    const auto& game = session.getGame();
    const auto& rules = game.getRoundRules();
    const auto* localPlayer = game.getLocalPlayer();

    const bool spectating =
        localPlayer && localPlayer->isEliminated();

    const Color panel{29, 29, 29, 255};
    const Color card{42, 42, 42, 255};
    const Color localCard{53, 53, 53, 255};
    const Color text{240, 240, 240, 255};
    const Color muted{165, 165, 165, 255};
    const Color accent{210, 210, 210, 255};
    const Color aliveColor{112, 218, 160, 255};
    const Color eliminatedColor{239, 168, 107, 255};
    const Color line{65, 65, 65, 255};

    const float padding = 16.0f;
    const float left = bounds.x + padding;
    const float width = bounds.width - 2.0f * padding;

    DrawRectangleRec(bounds, panel);

    // Header
    UI::drawText(
        "MATCH",
        left,
        bounds.y + 16.0f,
        14.0f,
        muted
    );

    UI::drawText(
        "Round ID " + std::to_string(game.getRoundId()),
        left,
        bounds.y + 38.0f,
        24.0f,
        text
    );

    const Rectangle statusBox{
        left,
        bounds.y + 76.0f,
        width,
        34.0f
    };

    DrawRectangleRounded(
        statusBox,
        0.2f,
        8,
        spectating
            ? Color{66, 47, 36, 255}
            : Color{32, 57, 48, 255}
    );

    UI::drawText(
        spectating ? "SPECTATING" : "IN PLAY",
        statusBox.x + 12.0f,
        statusBox.y + 8.0f,
        17.0f,
        spectating ? eliminatedColor : aliveColor
    );

    // Roster: local player first, then stable player-ID order.
    std::vector<const Player*> players;
    players.reserve(game.getPlayers().size());

    int aliveCount = 0;

    for (const auto& entry : game.getPlayers())
    {
        const auto& player = entry.second;
        players.push_back(&player);

        if (player.hasPosition() && !player.isEliminated())
            ++aliveCount;
    }

    const auto localId = game.getLocalPlayerId();

    std::sort(
        players.begin(),
        players.end(),
        [localId](const Player* first, const Player* second)
        {
            const bool firstLocal = first->getId() == localId;
            const bool secondLocal = second->getId() == localId;

            if (firstLocal != secondLocal)
                return firstLocal;

            return first->getId() < second->getId();
        }
    );

    float y = bounds.y + 130.0f;

    UI::drawText("PLAYERS", left, y, 14.0f, muted);

    const std::string aliveLabel =
        std::to_string(aliveCount) + " / " +
        std::to_string(players.size()) + " alive";

    UI::drawText(
        aliveLabel,
        left + width - UI::measureText(aliveLabel, 14.0f),
        y,
        14.0f,
        text
    );

    y += 26.0f;

    for (const Player* player : players)
    {
        const bool isLocal = player->getId() == localId;
        const bool eliminated = player->isEliminated();

        const Rectangle row{left, y, width, 48.0f};

        DrawRectangleRounded(
            row,
            0.16f,
            8,
            isLocal ? localCard : card
        );

        // Player color matches the arena.
        DrawRectangleRounded(
            Rectangle{left + 10.0f, y + 14.0f, 20.0f, 20.0f},
            0.15f,
            4,
            Fade(
                playerColor(player->getColor()),
                eliminated ? 0.4f : 1.0f
            )
        );

        std::string name = player->getName();

        if (isLocal)
            name += " (you)";

        UI::drawFittedText(
            Rectangle{
                left + 40.0f,
                y + 6.0f,
                width - 50.0f,
                20.0f
            },
            name,
            17.0f,
            eliminated ? muted : text
        );

        std::string status = eliminated ? "Eliminated" : "Alive";

        if (game.getLeaderId() &&
            *game.getLeaderId() == player->getId())
        {
            status += "  /  Leader";
        }

        UI::drawText(
            status,
            left + 40.0f,
            y + 29.0f,
            12.0f,
            eliminated ? eliminatedColor : aliveColor
        );

        y += 54.0f;
    }

    // Ability cards follow the roster instead of leaving a large gap.
    y += 10.0f;

    UI::drawText("YOUR ABILITIES", left, y, 14.0f, muted);
    y += 24.0f;

    const float gap = 10.0f;
    const float statWidth = (width - gap) / 2.0f;

    const auto drawStat = [&](Rectangle rectangle, const char* label, std::uint32_t value)
    {
        DrawRectangleRounded(rectangle, 0.15f, 8, card);

        UI::drawText(
            std::to_string(value),
            rectangle.x + 12.0f,
            rectangle.y + 7.0f,
            28.0f,
            accent
        );

        UI::drawText(
            label,
            rectangle.x + 12.0f,
            rectangle.y + 42.0f,
            13.0f,
            muted
        );
    };

    drawStat(
        Rectangle{left, y, statWidth, 66.0f},
        "Bomb limit",
        rules.bombCapacity
    );

    drawStat(
        Rectangle{left + statWidth + gap, y, statWidth, 66.0f},
        "Blast range",
        rules.blastRange
    );

    // Controls stay anchored near the bottom.
    const float controlsY = bounds.y + bounds.height - 126.0f;

    DrawRectangleRec(
        Rectangle{left, controlsY - 10.0f, width, 1.0f},
        line
    );

    const auto drawControl = [&](float rowY,
                                 const char* key,
                                 const char* action,
                                 bool enabled)
    {
        const Rectangle keyBox{left, rowY, 62.0f, 23.0f};

        DrawRectangleRounded(keyBox, 0.15f, 6, card);

        UI::drawCenteredText(
            keyBox,
            key,
            12,
            enabled ? text : muted
        );

        UI::drawText(
            action,
            left + 74.0f,
            rowY + 3.0f,
            15.0f,
            enabled ? text : muted
        );
    };

    drawControl(controlsY, "Arrows", "Move", !spectating);
    drawControl(controlsY + 29.0f, "Space", "Place bomb", !spectating);
    drawControl(controlsY + 58.0f, "Esc", "Disconnect", true);

    // A fixed message area prevents notifications covering other content.
    const Rectangle messageBounds{
        left,
        bounds.y + bounds.height - 30.0f,
        width,
        20.0f
    };

    if (!actionMessage.empty() &&
        GetTime() < actionMessageExpiresAt)
    {
        UI::drawFittedText(
            messageBounds,
            actionMessage,
            13.0f,
            eliminatedColor
        );
    }
    else if (spectating)
    {
        UI::drawFittedText(
            messageBounds,
            "You are out. Watching the remaining players.",
            13.0f,
            muted
        );
    }
}