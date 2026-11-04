#include "LobbyScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace
{
    constexpr Color Background{20, 20, 20, 255};
    constexpr Color Panel{29, 29, 29, 255};
    constexpr Color Surface{42, 42, 42, 255};
    constexpr Color LocalSurface{53, 53, 53, 255};
    constexpr Color Text{240, 240, 240, 255};
    constexpr Color Muted{165, 165, 165, 255};
    constexpr Color Accent{210, 210, 210, 255};

    constexpr Color ReadyColor{112, 218, 160, 255};
    constexpr Color Warning{239, 168, 107, 255};

    Rectangle rosterRectangle()
    {
        return {
            32.0f,
            126.0f,
            static_cast<float>(GetScreenWidth()) - 392.0f,
            static_cast<float>(GetScreenHeight()) - 158.0f
        };
    }

    Rectangle actionsRectangle()
    {
        return {
            static_cast<float>(GetScreenWidth()) - 336.0f,
            126.0f,
            304.0f,
            static_cast<float>(GetScreenHeight()) - 158.0f
        };
    }

    Rectangle readyButtonRectangle()
    {
        const auto panel = actionsRectangle();

        return {
            panel.x + 20.0f,
            panel.y + 112.0f,
            panel.width - 40.0f,
            48.0f
        };
    }

    Rectangle startButtonRectangle()
    {
        const auto panel = actionsRectangle();

        return {
            panel.x + 20.0f,
            panel.y + 284.0f,
            panel.width - 40.0f,
            48.0f
        };
    }

    Rectangle disconnectButtonRectangle()
    {
        const auto panel = actionsRectangle();

        return {
            panel.x + 20.0f,
            panel.y + panel.height - 64.0f,
            panel.width - 40.0f,
            44.0f
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

    UI::Theme buttonTheme(bool primary)
    {
        UI::Theme theme;

        theme.button = primary
            ? Color{72, 72, 72, 255}
            : Surface;

        theme.buttonHovered = primary
            ? Color{90, 90, 90, 255}
            : Color{56, 56, 56, 255};

        theme.buttonText = Text;
        theme.disabled = Color{35, 35, 35, 255};
        theme.fontSize = 16;

        return theme;
    }

    bool everyoneReady(const Game& game)
    {
        if (game.getPlayers().size() < 2)
            return false;

        return std::all_of(
            game.getPlayers().begin(),
            game.getPlayers().end(),
            [](const auto& entry)
            {
                return entry.second.isReady();
            }
        );
    }

    bool canStartMatch(const GameSession& session)
    {
        return
            session.getState() == GameSession::State::Joined &&
            session.getGame().isLocalPlayerLeader() &&
            everyoneReady(session.getGame());
    }
}

LobbyScene::LobbyScene(GameSession& session)
    : session(session)
{
}

std::optional<SceneTransition> LobbyScene::update()
{
    const auto state = session.getState();

    if (state == GameSession::State::Failed)
    {
        std::string error = session.getError();
        session.disconnect();

        return SceneTransition{
            SceneType::Menu,
            std::move(error)
        };
    }

    if (state == GameSession::State::Disconnected)
    {
        return SceneTransition{
            SceneType::Menu,
            "Disconnected from server."
        };
    }

    if (state == GameSession::State::Playing)
    {
        return SceneTransition{SceneType::Playing, {}};
    }

    if (state == GameSession::State::Finished)
    {
        return SceneTransition{SceneType::Results, {}};
    }

    if (IsKeyPressed(KEY_ESCAPE) ||
        UI::isClicked(disconnectButtonRectangle()))
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, {}};
    }

    if (state != GameSession::State::Joined)
        return std::nullopt;

    const auto* localPlayer = session.getGame().getLocalPlayer();

    if (localPlayer &&
        (IsKeyPressed(KEY_R) ||
         UI::isClicked(readyButtonRectangle())))
    {
        session.setReady(!localPlayer->isReady());

        // Avoid sending Ready and Start in the same frame.
        return std::nullopt;
    }

    if (canStartMatch(session) &&
        (IsKeyPressed(KEY_ENTER) ||
         UI::isClicked(startButtonRectangle())))
    {
        session.requestGameStart();
    }

    return std::nullopt;
}

void LobbyScene::render() const
{
    const auto& game = session.getGame();
    const auto state = session.getState();
    const auto* localPlayer = game.getLocalPlayer();

    const bool joined = state == GameSession::State::Joined;
    const bool localReady = localPlayer && localPlayer->isReady();
    const bool leader = game.isLocalPlayerLeader();

    DrawRectangle(
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight(),
        Background
    );

    UI::drawText("MULTIPLAYER", 32.0f, 24.0f, 14.0f, Muted);
    UI::drawText("Lobby", 32.0f, 48.0f, 34.0f, Text);

    UI::drawText(
        "Ready up. The leader starts the round.",
        32.0f,
        91.0f,
        16.0f,
        Muted
    );

    const Rectangle roster = rosterRectangle();
    const Rectangle actions = actionsRectangle();

    DrawRectangleRounded(roster, 0.025f, 8, Panel);
    DrawRectangleRounded(actions, 0.04f, 8, Panel);

    // Stable roster order.
    std::vector<const Player*> players;
    players.reserve(game.getPlayers().size());

    int readyCount = 0;

    for (const auto& entry : game.getPlayers())
    {
        players.push_back(&entry.second);

        if (entry.second.isReady())
            ++readyCount;
    }

    std::sort(
        players.begin(),
        players.end(),
        [](const Player* first, const Player* second)
        {
            return first->getId() < second->getId();
        }
    );

    UI::drawText(
        "PLAYERS",
        roster.x + 20.0f,
        roster.y + 22.0f,
        15.0f,
        Muted
    );

    const std::string countLabel =
        std::to_string(readyCount) + " / " +
        std::to_string(players.size()) + " ready";

    UI::drawText(
        countLabel,
        roster.x + roster.width - 20.0f -
            UI::measureText(countLabel, 15.0f),
        roster.y + 22.0f,
        15.0f,
        Text
    );

    float y = roster.y + 62.0f;

    for (const Player* player : players)
    {
        const bool isLocal =
            player->getId() == game.getLocalPlayerId();

        const bool isLeader =
            game.getLeaderId() &&
            *game.getLeaderId() == player->getId();

        const Rectangle row{
            roster.x + 20.0f,
            y,
            roster.width - 40.0f,
            68.0f
        };

        DrawRectangleRounded(
            row,
            0.12f,
            8,
            isLocal ? LocalSurface : Surface
        );

        DrawRectangleRounded(
            Rectangle{
                row.x + 14.0f,
                row.y + 18.0f,
                32.0f,
                32.0f
            },
            0.15f,
            6,
            playerColor(player->getColor())
        );

        std::string name = player->getName().empty()
            ? "Player " + std::to_string(player->getId())
            : player->getName();

        if (isLocal)
            name += " (you)";

        UI::drawFittedText(
            Rectangle{
                row.x + 60.0f,
                row.y + 10.0f,
                row.width - 200.0f,
                25.0f
            },
            name,
            19.0f,
            Text
        );

        UI::drawText(
            isLeader ? "Lobby leader" : "Player",
            row.x + 60.0f,
            row.y + 40.0f,
            13.0f,
            isLeader ? Accent : Muted
        );

        const Rectangle badge{
            row.x + row.width - 124.0f,
            row.y + 19.0f,
            110.0f,
            30.0f
        };

        DrawRectangleRounded(
            badge,
            0.2f,
            6,
            player->isReady()
                ? Color{32, 57, 48, 255}
                : Color{51, 51, 51, 255}
        );

        UI::drawCenteredText(
            badge,
            player->isReady() ? "READY" : "NOT READY",
            13,
            player->isReady() ? ReadyColor : Muted
        );

        y += 80.0f;
    }

    if (players.empty())
    {
        UI::drawText(
            "Waiting for players...",
            roster.x + 20.0f,
            y,
            17.0f,
            Muted
        );
    }

    // Reserve the roster footer for reset/error notices.
    const std::string footer = session.getNotice().empty()
        ? "Your player color is also used in the arena."
        : session.getNotice();

    UI::drawFittedText(
        Rectangle{
            roster.x + 20.0f,
            roster.y + roster.height - 48.0f,
            roster.width - 40.0f,
            28.0f
        },
        footer,
        14.0f,
        session.getNotice().empty() ? Muted : Warning
    );

    // Local readiness.
    const float left = actions.x + 20.0f;

    UI::drawText(
        "YOUR STATUS",
        left,
        actions.y + 22.0f,
        14.0f,
        Muted
    );

    UI::drawText(
        localReady ? "Ready to play" : "Not ready yet",
        left,
        actions.y + 50.0f,
        23.0f,
        localReady ? ReadyColor : Text
    );

    UI::drawText(
        "Press R or use the button below.",
        left,
        actions.y + 83.0f,
        14.0f,
        Muted
    );

    UI::drawButton(
        readyButtonRectangle(),
        localReady ? "CANCEL READY  [R]" : "READY UP  [R]",
        joined && localPlayer != nullptr,
        buttonTheme(!localReady)
    );

    DrawRectangleRec(
        Rectangle{
            left,
            actions.y + 182.0f,
            actions.width - 40.0f,
            1.0f
        },
        Surface
    );

    // Match preparation and start requirements.
    const char* phaseTitle;
    const char* phaseHint;

    if (state == GameSession::State::PreparingMatch)
    {
        phaseTitle = "Preparing arena";
        phaseHint = "Receiving the server's round setup.";
    }
    else if (state == GameSession::State::WaitingForMatchStart)
    {
        phaseTitle = "Starting soon";
        phaseHint = "Waiting for everyone to finish loading.";
    }
    else if (!joined)
    {
        phaseTitle = "Connecting";
        phaseHint = "Waiting for the lobby.";
    }
    else if (players.size() < 2)
    {
        phaseTitle = "Waiting for players";
        phaseHint = "At least two players are needed.";
    }
    else if (!everyoneReady(game))
    {
        phaseTitle = "Waiting for readiness";
        phaseHint = "Everyone must be ready to start.";
    }
    else
    {
        phaseTitle = "Everyone is ready";
        phaseHint = leader
            ? "Start the round when you are ready."
            : "The leader can now start the round.";
    }

    UI::drawFittedText(
        Rectangle{
            left,
            actions.y + 202.0f,
            actions.width - 40.0f,
            30.0f
        },
        phaseTitle,
        21.0f,
        Text
    );

    UI::drawFittedText(
        Rectangle{
            left,
            actions.y + 241.0f,
            actions.width - 40.0f,
            22.0f
        },
        phaseHint,
        14.0f,
        Muted
    );

    const char* startLabel =
        !joined ? "PREPARING..." :
        leader ? "START ROUND  [Enter]" :
                 "LEADER STARTS THE ROUND";

    UI::drawButton(
        startButtonRectangle(),
        startLabel,
        canStartMatch(session),
        buttonTheme(true)
    );

    UI::drawButton(
        disconnectButtonRectangle(),
        "DISCONNECT  [Esc]",
        true,
        buttonTheme(false)
    );
}