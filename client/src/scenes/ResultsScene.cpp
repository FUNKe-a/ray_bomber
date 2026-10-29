#include "ResultsScene.hpp"

#include "GameSession.hpp"

#include <raylib.h>

#include <string>
#include <utility>

ResultsScene::ResultsScene(GameSession& session)
    : session(session)
{
}

std::optional<SceneTransition> ResultsScene::update()
{
    const auto state = session.getState();

    if (state == GameSession::State::Failed)
    {
        std::string error = session.getError();
        session.disconnect();

        return SceneTransition{SceneType::Menu, std::move(error)};
    }

    if (state == GameSession::State::Disconnected)
    {
        return SceneTransition{SceneType::Menu, "Disconnected from server."
        };
    }

    if (state == GameSession::State::Joined ||
        state == GameSession::State::PreparingMatch ||
        state == GameSession::State::WaitingForMatchStart)
    {
        return SceneTransition{SceneType::Lobby, {}};
    }

    if (state == GameSession::State::Playing)
    {
        return SceneTransition{SceneType::Playing, {}};
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, {}};
    }

    if (state == GameSession::State::Finished &&
        IsKeyPressed(KEY_ENTER))
    {
        session.requestReturnToLobby();
    }

    return std::nullopt;
}

void ResultsScene::render() const
{
    const auto& result = session.getRoundResult();

    if (!result)
    {
        return;
    }

    DrawText("ROUND RESULTS", 40, 50, 30, DARKGRAY);

    const std::string outcome =
        result->outcome == RoundOutcome::Draw
            ? "Draw!"
            : "Winner: " + result->winnerName;

    DrawText(outcome.c_str(), 40, 120, 20, DARKBLUE);

    const char* hint;

    if (session.isWaitingForLobbyReset())
        hint = "Waiting for server...";
    else if (session.getGame().isLocalPlayerLeader())
        hint = "Press Enter to return everyone to the lobby";
    else
        hint = "Waiting for the leader to return to the lobby";

    DrawText(hint, 40, 190, 20, DARKGRAY);

    if (!session.getNotice().empty())
    {
        DrawText(
            session.getNotice().c_str(),
            40,
            235,
            20,
            MAROON
        );
    }

    DrawText(
        "Escape: disconnect",
        40,
        GetScreenHeight() - 50,
        20,
        GRAY
    );
}