#include "Application.hpp"

#include "Scene.hpp"

#include <raylib.h>

#include <utility>

namespace
{
    constexpr int ScreenWidth = 720;
    constexpr int ScreenHeight = 624;
    constexpr int TargetFramesPerSecond = 60;
}

Application::Application()
    : factory(session, settings)
{
}

Application::~Application() = default;

int Application::run()
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(ScreenWidth, ScreenHeight, "Ray Bomber");
    SetTargetFPS(TargetFramesPerSecond);

    scene = factory.create(SceneType::Menu);

    SetExitKey(KEY_NULL);

    while (!WindowShouldClose())
    {
        session.update();

        if (auto transition = scene->update())
        {
            auto nextScene = factory.create(
                transition->target,
                std::move(transition->errorMessage)
            );

            scene = std::move(nextScene);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene->render();

        EndDrawing();
    }

    scene.reset();
    session.disconnect();
    CloseWindow();

    return 0;
}
