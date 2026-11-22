#include "ResultsScene.hpp"

#include "GameSession.hpp"
#include "Controls.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <string>
#include <utility>

namespace
{
    constexpr Color Surface{42, 42, 42, 255};
    constexpr Color Text{240, 240, 240, 255};
    constexpr Color Muted{165, 165, 165, 255};
    constexpr Color Accent{210, 210, 210, 255};
    constexpr Color Victory{112, 218, 160, 255};
    constexpr Color Warning{239, 168, 107, 255};

    Rectangle resultsRectangle()
    {
        constexpr float width = 560.0f;
        constexpr float height = 480.0f;

        return {
            (GetScreenWidth() - width) / 2.0f,
            (GetScreenHeight() - height) / 2.0f,
            width,
            height
        };
    }

    Rectangle returnButtonRectangle()
    {
        const auto panel = resultsRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 320.0f,
            panel.width - 64.0f,
            48.0f
        };
    }

    Rectangle disconnectButtonRectangle()
    {
        const auto panel = resultsRectangle();

        return {
            panel.x + 32.0f,
            panel.y + 380.0f,
            panel.width - 64.0f,
            42.0f
        };
    }

    UI::Theme primaryButtonTheme()
    {
        UI::Theme theme;

        theme.button = Color{72, 72, 72, 255};
        theme.buttonHovered = Color{90, 90, 90, 255};
        theme.buttonText = Text;
        theme.disabled = Color{35, 35, 35, 255};
        theme.fontSize = 17;

        return theme;
    }

    UI::Theme secondaryButtonTheme()
    {
        UI::Theme theme;

        theme.button = Surface;
        theme.buttonHovered = Color{56, 56, 56, 255};
        theme.buttonText = Text;
        theme.disabled = Color{35, 35, 35, 255};
        theme.fontSize = 16;

        return theme;
    }
}

struct ResultsScene::Controls
{
    bool disconnectRequested = false;

    UI::Button returnToLobby;
    UI::Button disconnect;
    UI::Form form;

    explicit Controls(GameSession& session)
        : returnToLobby(
              "RETURN TO LOBBY",
              primaryButtonTheme(),
              [&session] { session.requestReturnToLobby(); }),
          disconnect(
              "DISCONNECT  [Esc]",
              secondaryButtonTheme(),
              [this] { disconnectRequested = true; }),
          form{returnToLobby, disconnect}
    {
        returnToLobby.setBounds(returnButtonRectangle());
        disconnect.setBounds(disconnectButtonRectangle());

        refresh(session);

        if (!returnToLobby.isEnabled())
            form.focus(disconnect);
    }

    void refresh(const GameSession& session)
    {
        const bool leader = session.getGame().isLocalPlayerLeader();
        const bool waiting = session.isWaitingForLobbyReset();

        returnToLobby.setEnabled(
            session.getState() == GameSession::State::Finished &&
            session.getRoundResult().has_value() &&
            leader &&
            !waiting
        );

        if (waiting)
            returnToLobby.setLabel("RETURNING...");
        else if (leader)
            returnToLobby.setLabel("RETURN TO LOBBY");
        else
            returnToLobby.setLabel("WAITING FOR LEADER");
    }
};

ResultsScene::ResultsScene(GameSession& session)
    : session(session),
      controls(std::make_unique<Controls>(session))
{
}

ResultsScene::~ResultsScene() = default;

Rectangle ResultsScene::panelBounds() const
{
    return resultsRectangle();
}

std::optional<SceneTransition> ResultsScene::update()
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

    controls->refresh(session);
    controls->form.update();

    if (controls->disconnectRequested)
    {
        session.disconnect();
        return SceneTransition{SceneType::Menu, {}};
    }

    controls->refresh(session);
    return std::nullopt;
}

void ResultsScene::drawContent(Rectangle panel) const
{
    const auto& result = session.getRoundResult();

    if (!result)
    {
        controls->form.draw();
        return;
    }

    const auto& game = session.getGame();
    const bool isLeader = game.isLocalPlayerLeader();
    const bool waiting = session.isWaitingForLobbyReset();

    const bool draw = result->outcome == RoundOutcome::Draw;

    const bool localWinner =
        !draw &&
        result->winnerId &&
        *result->winnerId == game.getLocalPlayerId();

    const Color outcomeColor =
        draw ? Accent : localWinner ? Victory : Text;

    const float contentX = panel.x + 32.0f;
    const float contentWidth = panel.width - 64.0f;

    UI::drawCenteredText(
        Rectangle{
            contentX,
            panel.y + 26.0f,
            contentWidth,
            24.0f
        },
        "ROUND COMPLETE",
        15,
        Muted
    );

    const char* headline =
        draw ? "DRAW" : localWinner ? "VICTORY!" : "ROUND OVER";

    UI::drawCenteredText(
        Rectangle{
            contentX,
            panel.y + 63.0f,
            contentWidth,
            54.0f
        },
        headline,
        38,
        outcomeColor
    );

    UI::drawCenteredText(
        Rectangle{
            contentX,
            panel.y + 127.0f,
            contentWidth,
            22.0f
        },
        "Round ID " + std::to_string(result->roundId),
        14,
        Muted
    );

    // Use the cached winner name so it survives a disconnect.
    const Rectangle outcomeBox{
        contentX,
        panel.y + 175.0f,
        contentWidth,
        86.0f
    };

    DrawRectangleRounded(
        outcomeBox,
        0.12f,
        8,
        Surface
    );

    UI::drawCenteredText(
        Rectangle{
            outcomeBox.x + 16.0f,
            outcomeBox.y + 10.0f,
            outcomeBox.width - 32.0f,
            20.0f
        },
        draw ? "NO SURVIVORS" : "WINNER",
        13,
        Muted
    );

    const std::string outcomeText =
        draw ? "Nobody survived this round" : result->winnerName;

    // Fit long names while keeping the text centered.
    const std::string fittedOutcome = UI::fitText(
        outcomeText,
        outcomeBox.width - 32.0f,
        24.0f
    );

    UI::drawCenteredText(
        Rectangle{
            outcomeBox.x + 16.0f,
            outcomeBox.y + 36.0f,
            outcomeBox.width - 32.0f,
            36.0f
        },
        fittedOutcome,
        24,
        draw ? Text : Victory
    );

    const char* hint;

    if (waiting)
        hint = "Waiting for the server to return everyone...";
    else if (isLeader)
        hint = "You can return everyone to the lobby.";
    else
        hint = "Waiting for the leader to return to the lobby.";

    UI::drawCenteredText(
        Rectangle{
            contentX,
            panel.y + 280.0f,
            contentWidth,
            24.0f
        },
        hint,
        15,
        Muted
    );

    controls->form.draw();

    if (!session.getNotice().empty())
    {
        UI::drawFittedText(
            Rectangle{
                contentX,
                panel.y + 437.0f,
                contentWidth,
                22.0f
            },
            session.getNotice(),
            14.0f,
            Warning
        );
    }
}