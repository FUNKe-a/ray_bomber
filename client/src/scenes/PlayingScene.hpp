#pragma once

#include "Scene.hpp"
#include "GameRenderer.hpp"
#include "GameplayInput.hpp"
#include "Game.hpp"

#include <string>
#include <optional>

class GameSession;

class PlayingScene final : public Scene
{
public:
    explicit PlayingScene(GameSession& session);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    void onGameNotification(const GameNotification& notification);
    void renderHud(Rectangle bounds) const;

    GameSession& session;
    GameplayInput input;
    GameRenderer renderer;

    std::string actionMessage;
    double actionMessageExpiresAt = 0.0;

    Game::PresentationSubscription presentationSubscription;
};