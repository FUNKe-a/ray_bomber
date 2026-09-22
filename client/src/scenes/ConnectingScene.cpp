#include "ConnectingScene.hpp"

#include "MenuScene.hpp"
#include "PlayingScene.hpp"
#include "Client.hpp"

#include <raylib.h>

#include <memory>
#include <utility>

ConnectingScene::ConnectingScene(
    Client& client,
    std::string serverAddress,
    std::string port
)
    : client(client),
      serverAddress(std::move(serverAddress)),
      port(std::move(port))
{
}

std::unique_ptr<ApplicationState> ConnectingScene::update()
{
    if (client.isConnected())
    {
        client.startReceiving();
        return std::make_unique<PlayingScene>(
            client, 
            std::move(serverAddress),
            std::move(port)    
        );
    }

    if (client.getConnectionState() == Client::ConnectionState::Failed)
    {
        return std::make_unique<MenuScene>(
            client,
            std::move(serverAddress),
            std::move(port),
            client.getError()
        );
    }

    return nullptr;
}

void ConnectingScene::render() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const char* text = "Connecting...";

    DrawText(
        text,
        360 - MeasureText(text, 30) / 2,
        220,
        30,
        DARKGRAY
    );

    EndDrawing();
}
