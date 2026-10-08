#pragma once

#include <string>

enum class SceneType
{
    Menu,
    Settings,
    Connecting,
    Lobby
};

struct SceneTransition
{
    SceneType target;
    std::string errorMessage;
};