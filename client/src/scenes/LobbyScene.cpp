#include "LobbyScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"
#include "MenuScene.hpp"

#include <raylib.h>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

namespace
{
    Rectangle disconnectButton()
    {
        return {
            static_cast<float>(GetScreenWidth()) / 2.0f - 110.0f,
            static_cast<float>(GetScreenHeight()) - 70.0f,
            220.0f,
            44.0f
        };
    }

    Color toRaylibColor(PlayerColor color)
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
}

LobbyScene::LobbyScene(
    GameSession& session,
    std::string serverAddress,
    std::string port,
    std::string playerName
)
    : session(session),
      serverAddress(std::move(serverAddress)),
      port(std::move(port)),
      playerName(std::move(playerName))
{
}

std::unique_ptr<Scene> LobbyScene::returnToMenu(std::string error)
{
    return std::make_unique<MenuScene>(
        session,
        std::move(serverAddress),
        std::move(port),
        std::move(error),
        std::move(playerName)
    );
}

std::unique_ptr<Scene> LobbyScene::update()
{
    const auto state = session.getState();

    if (state == GameSession::State::Failed)
    {
        // Copy the error before disconnect() clears it.
        std::string error = session.getError();
        session.disconnect();

        return returnToMenu(std::move(error));
    }

    if (state == GameSession::State::Disconnected)
    {
        return returnToMenu("Disconnected from server.");
    }

    const bool disconnectClicked =
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
        CheckCollisionPointRec(
            GetMousePosition(),
            disconnectButton()
        );

    if (disconnectClicked || IsKeyPressed(KEY_ESCAPE))
    {
        session.disconnect();

        // Leaving intentionally is not an error.
        return returnToMenu("");
    }

    if (state == GameSession::State::Joined)
    {
        const Player* localPlayer =
            session.getGame().getLocalPlayer();

        if (localPlayer && IsKeyPressed(KEY_R))
        {
            session.setReady(!localPlayer->isReady());
        }
    }

    return nullptr;
}

void LobbyScene::render() const
{
    const Game& game = session.getGame();

    const char* title = "LOBBY";

    DrawText(
        title,
        (GetScreenWidth() - MeasureText(title, 36)) / 2,
        35,
        30,
        DARKGRAY
    );

    const char* hint = "Press R to toggle ready";

    DrawText(
        hint,
        (GetScreenWidth() - MeasureText(hint, 20)) / 2,
        85,
        20,
        GRAY
    );

    const int left = 40;
    const int width = GetScreenWidth() - 2 * left;
    const int statusX = left + width - 160;

    DrawText("COLOR", left + 12, 130, 20, GRAY);
    DrawText("PLAYER", left + 90, 130, 20, GRAY);
    DrawText("STATUS", statusX, 130, 20, GRAY);

    // unordered_map does not provide a stable display order.
    std::vector<std::uint32_t> ids;
    ids.reserve(game.getPlayers().size());

    for (const auto& entry : game.getPlayers())
    {
        ids.push_back(entry.first);
    }

    std::sort(ids.begin(), ids.end());

    int y = 160;

    for (const auto id : ids)
    {
        const Player* player = game.getPlayer(id);

        const bool isLocalPlayer =
            game.hasJoinedLobby() &&
            id == game.getLocalPlayerId();

        const Rectangle row{
            static_cast<float>(left),
            static_cast<float>(y),
            static_cast<float>(width),
            48.0f
        };

        DrawRectangleRec(row, LIGHTGRAY);

        if (isLocalPlayer)
        {
            DrawRectangleLinesEx(row, 2.0f, DARKBLUE);
        }

        // Show the assigned color independently of ready status.
        DrawRectangle(
            left + 20,
            y + 12,
            24,
            24,
            toRaylibColor(player->getColor())
        );

        DrawRectangleLines(
            left + 20,
            y + 12,
            24,
            24,
            DARKGRAY
        );

        std::string name = player->getName().empty()
            ? "Player " + std::to_string(id)
            : player->getName();

        const std::string suffix = isLocalPlayer ? " (you)" : "";

        // Keep long names from overlapping the status column.
        const int nameWidth = statusX - (left + 90) - 16;

        if (MeasureText((name + suffix).c_str(), 20) > nameWidth)
        {
            while (!name.empty() &&
                   MeasureText(
                       (name + "..." + suffix).c_str(), 20
                   ) > nameWidth)
            {
                name.pop_back();
            }

            name += "...";
        }

        const std::string label = name + suffix;

        DrawText(
            label.c_str(),
            left + 90,
            y + 14,
            20,
            DARKGRAY
        );

        DrawText(
            player->isReady() ? "READY" : "NOT READY",
            statusX,
            y + 14,
            20,
            player->isReady() ? DARKGREEN : DARKGRAY
        );

        y += 58;
    }

    if (ids.empty())
    {
        DrawText("Waiting for players...", left + 12, y, 20, GRAY);
    }

    const Rectangle button = disconnectButton();

    const bool hovered =
        CheckCollisionPointRec(GetMousePosition(), button);

    DrawRectangleRec(button, hovered ? RED : MAROON);

    const char* buttonText = "Disconnect";

    DrawText(
        buttonText,
        static_cast<int>(
            button.x +
            (button.width - MeasureText(buttonText, 20)) / 2
        ),
        static_cast<int>(button.y + 12),
        20,
        WHITE
    );
}