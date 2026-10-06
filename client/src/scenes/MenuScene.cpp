#include "MenuScene.hpp"

#include "ConnectingScene.hpp"
#include "Client.hpp"

#include <raylib.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace
{
    constexpr int ScreenWidth = 720;
    constexpr int InputWidth = 624;
    constexpr int InputHeight = 50;
    constexpr int ButtonWidth = 240;
    constexpr int ButtonHeight = 55;

    Rectangle serverAddressRectangle()
    {
        return {
            (ScreenWidth - InputWidth) / 2.0f,
            155.0f,
            static_cast<float>(InputWidth),
            static_cast<float>(InputHeight)
        };
    }

    Rectangle portRectangle()
    {
        return {
            (ScreenWidth - InputWidth) / 2.0f,
            245.0f,
            static_cast<float>(InputWidth),
            static_cast<float>(InputHeight)
        };
    }

    Rectangle connectButtonRectangle()
    {
        return {
            (ScreenWidth - ButtonWidth) / 2.0f,
            430.0f,
            static_cast<float>(ButtonWidth),
            static_cast<float>(ButtonHeight)
        };
    }

    Rectangle nameRectangle()
    {
        return {
            (ScreenWidth - InputWidth) / 2.0f, 335.0f,
            static_cast<float>(InputWidth), static_cast<float>(InputHeight)
        };
    }
}

MenuScene::MenuScene(Client& client)
    : MenuScene(client, "127.0.0.1", "6769", "")
{
}

MenuScene::MenuScene(
    Client& client,
    std::string serverAddress,
    std::string port,
    std::string errorMessage,
    std::string playerName
)
    : client(client),
      serverAddress(std::move(serverAddress)),
      port(std::move(port)),
      errorMessage(std::move(errorMessage)),
      playerName(std::move(playerName))
{
}

std::unique_ptr<Scene> MenuScene::update()
{
    const Vector2 mousePosition = GetMousePosition();
    const Rectangle addressRect = serverAddressRectangle();
    const Rectangle portRect = portRectangle();
    const Rectangle buttonRect = connectButtonRectangle();
    const Rectangle nameRect = nameRectangle();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (CheckCollisionPointRec(mousePosition, addressRect))
        {
            activeField = InputField::ServerAddress;
        }
        else if (CheckCollisionPointRec(mousePosition, portRect))
        {
            activeField = InputField::Port;
        }
        else if (CheckCollisionPointRec(mousePosition, nameRect))
        {
            activeField = InputField::PlayerName;
        }
    }

    handleTextInput();

    if (!CheckCollisionPointRec(mousePosition, buttonRect) ||
        !IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
        !validateConnectionDetails())
    {
        return nullptr;
    }

    const auto serverPort = static_cast<unsigned short>(
        std::stoi(port)
    );

    client.connect(serverAddress, serverPort);

    return std::make_unique<ConnectingScene>(
        client,
        serverAddress,
        port,
        playerName
    );
}

void MenuScene::handleTextInput()
{
    int character = GetCharPressed();

    while (character > 0)
    {
        if (activeField == InputField::ServerAddress)
        {
            const bool isAddressCharacter =
                (character >= '0' && character <= '9') ||
                (character >= 'a' && character <= 'z') ||
                (character >= 'A' && character <= 'Z') ||
                character == '.' ||
                character == '-' ||
                character == ':';

            if (isAddressCharacter && serverAddress.size() < 255)
            {
                serverAddress += static_cast<char>(character);
            }
        }
        else if (activeField == InputField::PlayerName)
        {
            // Initial UI accepts printable ASCII, which is valid UTF-8.
            if (character >= 32 && character <= 126 && playerName.size() < 32)
                playerName += static_cast<char>(character);
        }
        else if (character >= '0' &&
                 character <= '9' &&
                 port.size() < 5)
        {
            port += static_cast<char>(character);
        }

        character = GetCharPressed();
    }

    if (!IsKeyPressed(KEY_BACKSPACE))
    {
        return;
    }

    std::string& value = activeField == InputField::ServerAddress
        ? serverAddress
        : activeField == InputField::Port ? port : playerName;

    if (!value.empty())
    {
        value.pop_back();
    }
}

bool MenuScene::validateConnectionDetails()
{
    if (playerName.empty() || playerName.find_first_not_of(' ') == std::string::npos)
    {
        errorMessage = "Enter a player name.";
        return false;
    }
    if (serverAddress.empty())
    {
        errorMessage = "Enter a server address.";
        return false;
    }

    if (port.empty())
    {
        errorMessage = "Enter a server port.";
        return false;
    }

    try
    {
        const int portNumber = std::stoi(port);

        if (portNumber < 1 || portNumber > 65535)
        {
            errorMessage = "Port must be between 1 and 65535.";
            return false;
        }
    }
    catch (const std::exception&)
    {
        errorMessage = "Invalid port.";
        return false;
    }

    errorMessage.clear();
    return true;
}

void MenuScene::render() const
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const char* title = "RAY BOMBER";

    DrawText(
        title,
        ScreenWidth / 2 - MeasureText(title, 40) / 2,
        55,
        40,
        DARKGRAY
    );

    const Rectangle addressRect = serverAddressRectangle();
    const Rectangle portRect = portRectangle();
    const Rectangle buttonRect = connectButtonRectangle();

    DrawText(
        "Server Address",
        static_cast<int>(addressRect.x),
        static_cast<int>(addressRect.y) - 28,
        20,
        GRAY
    );

    DrawText(
        "Port",
        static_cast<int>(portRect.x),
        static_cast<int>(portRect.y) - 28,
        20,
        GRAY
    );

    DrawRectangleRec(addressRect, LIGHTGRAY);
    DrawRectangleRec(portRect, LIGHTGRAY);

    const Rectangle nameRect = nameRectangle();
    DrawText("Player Name", static_cast<int>(nameRect.x),
             static_cast<int>(nameRect.y) - 28, 20, GRAY);
    DrawRectangleRec(nameRect, LIGHTGRAY);
    DrawRectangleLinesEx(nameRect, 2.0f,
        activeField == InputField::PlayerName ? BLUE : GRAY);
    DrawText(playerName.c_str(), static_cast<int>(nameRect.x) + 15,
             static_cast<int>(nameRect.y) + 14, 20, DARKGRAY);

    DrawRectangleLinesEx(
        addressRect,
        2.0f,
        activeField == InputField::ServerAddress ? BLUE : GRAY
    );

    DrawRectangleLinesEx(
        portRect,
        2.0f,
        activeField == InputField::Port ? BLUE : GRAY
    );

    DrawText(
        serverAddress.c_str(),
        static_cast<int>(addressRect.x) + 15,
        static_cast<int>(addressRect.y) + 14,
        20,
        DARKGRAY
    );

    DrawText(
        port.c_str(),
        static_cast<int>(portRect.x) + 15,
        static_cast<int>(portRect.y) + 14,
        20,
        DARKGRAY
    );

    const bool hovered = CheckCollisionPointRec(
        GetMousePosition(),
        buttonRect
    );

    DrawRectangleRec(buttonRect, hovered ? BLUE : DARKBLUE);

    const char* buttonText = "CONNECT";

    DrawText(
        buttonText,
        static_cast<int>(
            buttonRect.x +
            (buttonRect.width - MeasureText(buttonText, 20)) / 2
        ),
        static_cast<int>(buttonRect.y) + 16,
        20,
        WHITE
    );

    if (!errorMessage.empty())
    {
        DrawText(
            errorMessage.c_str(),
            ScreenWidth / 2 -
                MeasureText(errorMessage.c_str(), 20) / 2,
            515,
            20,
            RED
        );
    }

    EndDrawing();
}
