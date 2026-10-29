#pragma once

#include "Scene.hpp"

class GameSession;

class ResultsScene final : public Scene
{
public:
    explicit ResultsScene(GameSession& session);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    GameSession& session;
};