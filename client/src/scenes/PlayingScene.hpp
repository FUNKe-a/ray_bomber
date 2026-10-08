#pragma once

#include "Scene.hpp"

#include "GameRenderer.hpp"

#include <optional>

class GameSession;

class PlayingScene final : public Scene
{
public:
    explicit PlayingScene(GameSession& session);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    GameSession& session;
    GameRenderer renderer;
};