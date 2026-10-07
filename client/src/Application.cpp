#include "Application.hpp"

#include "Scene.hpp"
#include "MenuScene.hpp"

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
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(ScreenWidth, ScreenHeight, "Ray Bomber");
    SetTargetFPS(TargetFramesPerSecond);

    state = std::make_unique<MenuScene>(session);

    SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        session.update();

        auto nextState = state->update();

        if (nextState)
        {
            state = std::move(nextState);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        state->render();

        EndDrawing();
    }

    state.reset();
    session.disconnect();
    CloseWindow();

    return 0;
}
