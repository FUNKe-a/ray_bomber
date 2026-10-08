#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;

class ConnectingScene final : public Scene
{
public:
    ConnectingScene(GameSession& session);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    GameSession& session;
};
