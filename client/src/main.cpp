#include <iostream>

#include <raylib.h>
#include "Game.hpp"
#include "Renderer.hpp"
#include "Client.hpp"
#include "Protocol.hpp"

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
    if (!client.connect("127.0.0.1", 6769))
    {
        return 1;
    }

    client.startReceiving();

    while (!WindowShouldClose())
    {
        client.poll();

        Protocol::Message message;
        while (client.receive(message))
        {
            game.handleMessage(message);
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        {
            game.movePlayer(Direction::Up);
            client.send(
                Protocol::createMoveRequest(
                    Protocol::Direction::Up
                )
            );
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        {
            game.movePlayer(Direction::Down);
            client.send(
                Protocol::createMoveRequest(
                    Protocol::Direction::Down
                )
            );
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
        {
            game.movePlayer(Direction::Left);
            client.send(
                Protocol::createMoveRequest(
                    Protocol::Direction::Left
                )
            );
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        {
            game.movePlayer(Direction::Right);
            client.send(
                Protocol::createMoveRequest(
                    Protocol::Direction::Right
                )
            );
        }

        renderer.render(game);
    }

    client.disconnect();

    CloseWindow();

    return 0;
}
