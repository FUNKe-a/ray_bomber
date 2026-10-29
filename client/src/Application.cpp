#include "Application.hpp"

#include "Scene.hpp"
#include "UI.hpp"

#include <string>
#include <raylib.h>
#include <utility>

namespace
{
    constexpr int ScreenWidth = 1040;
    constexpr int ScreenHeight = 656;
    constexpr int TargetFramesPerSecond = 60;
}

Application::Application()
    : factory(session, settings)
{
}

Application::~Application() = default;

int Application::run()
{
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(ScreenWidth, ScreenHeight, "Ray Bomber");
    
    const std::string fontPath = std::string(GetApplicationDirectory()) + "assets/fonts/ui.ttf";
    if (!UI::loadFont(fontPath.c_str()))
    {
        TraceLog(LOG_ERROR, "Could not load UI font: %s", fontPath.c_str());
        CloseWindow();
        return 1;
    }
    
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
        ClearBackground(Color{20, 20, 20, 255});

        scene->render();

        EndDrawing();
    }

    scene.reset();
    session.disconnect();

    UI::unloadFont();
    CloseWindow();

    return 0;
}
