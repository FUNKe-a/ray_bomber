#pragma once

#include "Scene.hpp"
#include "Game.hpp"

#include <string>
#include <optional>

class GameSession;
class GameFacade;
struct Rectangle;

class PlayingScene final : public Scene
{
public:
    PlayingScene(GameSession& session, GameFacade& facade);

    std::optional<SceneTransition> update() override;
    void render() const override;

private:
    void onGameNotification(const GameNotification& notification);
    void renderHud(Rectangle bounds) const;

    GameSession& session;
    GameFacade& facade;

    std::string actionMessage;
    double actionMessageExpiresAt = 0.0;

    Game::PresentationSubscription presentationSubscription;
};