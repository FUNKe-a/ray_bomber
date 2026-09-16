#include "raylib.h"

#include "game/Game.hpp"
#include "rendering/Renderer.hpp"

int main()
{
    constexpr int tileSize = Renderer::TileSize;

    const int screenWidth = Map::Width * tileSize;
    const int screenHeight = Map::Height * tileSize;

    InitWindow(
        screenWidth,
        screenHeight,
        "Bomberman"
    );

    SetTargetFPS(60);

    Game game;
    Renderer renderer;

    while (!WindowShouldClose())
    {
        // Input
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            game.movePlayer(Direction::Up);
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            game.movePlayer(Direction::Down);
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        {
            game.movePlayer(Direction::Left);
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        {
            game.movePlayer(Direction::Right);
        }

        // Render
        renderer.render(game);
    }

    CloseWindow();

    return 0;
}