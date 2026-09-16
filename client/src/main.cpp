#include <raylib.h>
#include "Game.hpp"
#include "Renderer.hpp"
#include "Client.hpp"

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

    Client client;
    client.connect("127.0.0.1", 6769);
    client.send("CLIENT_CONNECTED\n");

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            game.movePlayer(Direction::Up);
            client.send("MOVE UP\n");
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            game.movePlayer(Direction::Down);
            client.send("MOVE DOWN\n");
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        {
            game.movePlayer(Direction::Left);
            client.send("MOVE LEFT\n");
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        {
            game.movePlayer(Direction::Right);
            client.send("MOVE RIGHT\n");
        }

        renderer.render(game);
    }

    client.send("CLIENT_DISCONNECTED\n");
    client.disconnect();

    CloseWindow();

    return 0;
}
