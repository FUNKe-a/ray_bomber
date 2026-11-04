#include "GameplayInput.hpp"

#include <raylib.h>

GameplayInput::GameplayInput()
    : bindings{{
        {
            KEY_UP,
            std::make_unique<MoveCommand>(Direction::Up)
        },
        {
            KEY_DOWN,
            std::make_unique<MoveCommand>(Direction::Down)
        },
        {
            KEY_LEFT,
            std::make_unique<MoveCommand>(Direction::Left)
        },
        {
            KEY_RIGHT,
            std::make_unique<MoveCommand>(Direction::Right)
        },
        {
            KEY_SPACE,
            std::make_unique<PlaceBombCommand>()
        }
    }}
{
}

void GameplayInput::handleInput(GameSession& session) const
{
    for (const auto& binding : bindings)
    {
        if (IsKeyPressed(binding.key))
        {
            TraceLog(LOG_INFO, "COMMAND: %s", binding.command->getName());
            binding.command->execute(session);
        }
    }
}