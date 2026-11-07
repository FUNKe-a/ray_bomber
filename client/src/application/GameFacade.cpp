#include "GameFacade.hpp"
#include "GameSession.hpp"

#include <raylib.h>

GameFacade::GameFacade(GameSession& session)
    : session(session)
{
}

void GameFacade::update()
{
    session.update();
}

void GameFacade::handleGameplayInput() const
{
    input.handleInput(session);
}

void GameFacade::renderGame(Rectangle viewport) const
{
    renderer.render(session.getGame(), viewport);
}