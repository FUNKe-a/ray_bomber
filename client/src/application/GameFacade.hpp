#pragma once

#include "GameplayInput.hpp"
#include "GameRenderer.hpp"

class GameSession;
struct Rectangle;

class GameFacade
{
public:
    explicit GameFacade(GameSession& session);

    void update();
    void handleGameplayInput() const;
    void renderGame(Rectangle viewport) const;

private:
    GameSession& session;
    GameplayInput input;
    GameRenderer renderer;
};