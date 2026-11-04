#pragma once

#include "Scene.hpp"

#include <memory>
#include <string>

class GameSession;

class LobbyScene final : public Scene
{
public:
    LobbyScene(GameSession& session);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    GameSession& session;
};
