#pragma once

#include <string>

enum class SceneType
{
    Menu,
    Settings,
    Connecting,
    Lobby,
    Playing,
    Results
};

struct SceneTransition
{
    SceneType target;
    std::string errorMessage;
};