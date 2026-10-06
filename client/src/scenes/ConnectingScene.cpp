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
    std::string port,
    std::string playerName
)
    : client(client),
      serverAddress(std::move(serverAddress)),
      port(std::move(port)),
      playerName(std::move(playerName))
{
}

std::unique_ptr<Scene> ConnectingScene::update()
{
    if (client.isConnected())
    {
        client.startReceiving();
        client.send(Protocol::createJoinLobbyRequest(playerName));
        std::cout << "Sent a JoinlobbyRequest with name " << playerName << std::endl;
        return std::make_unique<PlayingScene>(
            client, 
            std::move(serverAddress),
            std::move(port),
            std::move(playerName)
        );
    }

    if (client.getConnectionState() == Client::ConnectionState::Failed)
    {
        return std::make_unique<MenuScene>(
            client,
            std::move(serverAddress),
            std::move(port),
            client.getError(),
            std::move(playerName)
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
