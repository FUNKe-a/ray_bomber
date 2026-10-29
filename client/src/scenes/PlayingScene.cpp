#include "PlayingScene.hpp"

#include "GameSession.hpp"
#include "GameTypes.hpp"

#include <raylib.h>

#include <string>
#include <utility>


PlayingScene::PlayingScene(GameSession& session)
    : session(session)
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
}