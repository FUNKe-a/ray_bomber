#pragma once

#include "GameTypes.hpp"

class GameSession;

class GameplayCommand
{
public:
    virtual ~GameplayCommand() = default;

    virtual void execute(GameSession& session) const = 0;
    virtual const char* getName() const = 0;
};

class MoveCommand final : public GameplayCommand
{
public:
    explicit MoveCommand(Direction direction);

    void execute(GameSession& session) const override;
    const char* getName() const override;

private:
    Direction direction;
};

class PlaceBombCommand final : public GameplayCommand
{
public:
    void execute(GameSession& session) const override;
    const char* getName() const override;
};