#include "GameplayCommand.hpp"

#include "GameSession.hpp"

MoveCommand::MoveCommand(Direction direction)
    : direction(direction)
{
}

void MoveCommand::execute(GameSession& session) const
{
    session.move(direction);
}

const char* MoveCommand::getName() const
{
    switch (direction)
    {
        case Direction::Up:    return "Move Up";
        case Direction::Down:  return "Move Down";
        case Direction::Left:  return "Move Left";
        case Direction::Right: return "Move Right";
    }

    return "Move Unknown";
}

void PlaceBombCommand::execute(GameSession& session) const
{
    session.placeBomb();
}

const char* PlaceBombCommand::getName() const
{
    return "Place Bomb";
}