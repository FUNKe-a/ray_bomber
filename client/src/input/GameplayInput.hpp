#pragma once

#include "GameplayCommand.hpp"

#include <array>
#include <memory>

class GameSession;

class GameplayInput
{
public:
    GameplayInput();

    void handleInput(GameSession& session) const;

private:
    struct Binding
    {
        int key;
        std::unique_ptr<GameplayCommand> command;
    };

    std::array<Binding, 5> bindings;
};