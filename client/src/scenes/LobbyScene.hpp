#pragma once

#include "Scene.hpp"

#include <memory>

class GameSession;

class LobbyScene final : public Scene
{
public:
    LobbyScene(GameSession& session);
    ~LobbyScene() override;

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    struct Controls;

    GameSession& session;
    std::unique_ptr<Controls> controls;
};
