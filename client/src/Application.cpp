#include "Application.hpp"

#include "application/ApplicationState.hpp"
#include "scenes/MenuScene.hpp"

#include <raylib.h>

#include <utility>

namespace
{
    constexpr int ScreenWidth = 720;
    constexpr int ScreenHeight = 624;
    constexpr int TargetFramesPerSecond = 60;
}

Application::Application() = default;

Application::~Application() = default;

int Application::run()
{
    InitWindow(ScreenWidth, ScreenHeight, "Ray Bomber");
    SetTargetFPS(TargetFramesPerSecond);

    state = std::make_unique<MenuScene>(client);

    SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        client.poll();

        auto nextState = state->update();

        state->render();

        if (nextState)
        {
            state = std::move(nextState);
        }
    }

    client.disconnect();
    state.reset();
    CloseWindow();

    return 0;
}
