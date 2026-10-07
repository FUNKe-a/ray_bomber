#include "MenuScene.hpp"

#include "ConnectingScene.hpp"
#include "GameSession.hpp"
#include "UI.hpp"

#include <raylib.h>

#include <memory>
#include <stdexcept>
#include <utility>
#include <cstdint>

namespace
{
    constexpr int InputWidth = 624;
    constexpr int InputHeight = 50;
    constexpr int ButtonWidth = 240;
    constexpr int ButtonHeight = 55;

    Rectangle serverAddressRectangle()
    {
        return {
            (GetScreenWidth() - InputWidth) / 2.0f,
            155.0f,
            static_cast<float>(InputWidth),
            static_cast<float>(InputHeight)
        };
    }

    Rectangle portRectangle()
    {
        return {
            (GetScreenWidth() - InputWidth) / 2.0f,
            245.0f,
            static_cast<float>(InputWidth),
            static_cast<float>(InputHeight)
        };
    }

    Rectangle connectButtonRectangle()
    {
        return {
            (GetScreenWidth() - ButtonWidth) / 2.0f,
            430.0f,
            static_cast<float>(ButtonWidth),
            static_cast<float>(ButtonHeight)
        };
    }

    Rectangle nameRectangle()
    {
        return {
            (GetScreenWidth() - InputWidth) / 2.0f, 335.0f,
            static_cast<float>(InputWidth), static_cast<float>(InputHeight)
        };
    }
}

MenuScene::MenuScene(GameSession& session)
    : MenuScene(session, "127.0.0.1", "6769", "")
{
}

MenuScene::MenuScene(
    GameSession& session,
    std::string serverAddress,
    std::string port,
    std::string errorMessage,
    std::string playerName
)
    : session(session),
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

    const auto serverPort = static_cast<std::uint16_t>(std::stoi(port));

    session.connect(serverAddress, serverPort, playerName);

    return std::make_unique<ConnectingScene>(
        session,
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
    UI::drawCenteredText(
        Rectangle{0.0f, 55.0f, static_cast<float>(GetScreenWidth()), 40.0f},
        "RAY BOMBER",
        40,
        DARKGRAY
    );

    const Rectangle addressRect = serverAddressRectangle();
    const Rectangle portRect = portRectangle();
    const Rectangle nameRect = nameRectangle();
    const Rectangle buttonRect = connectButtonRectangle();

    DrawText(
        "Server Address",
        static_cast<int>(addressRect.x),
        static_cast<int>(addressRect.y) - 28,
        20,
        GRAY
    );

    UI::drawTextField(addressRect, serverAddress,
        activeField == InputField::ServerAddress
    );

    DrawText(
        "Port",
        static_cast<int>(portRect.x),
        static_cast<int>(portRect.y) - 28,
        20,
        GRAY
    );

    UI::drawTextField(portRect, port, activeField == InputField::Port);

    DrawText(
        "Player Name",
        static_cast<int>(nameRect.x),
        static_cast<int>(nameRect.y) - 28,
        20,
        GRAY
    );

    UI::drawTextField(nameRect, playerName, activeField == InputField::PlayerName);

    UI::drawButton(buttonRect, "CONNECT");

    if (!errorMessage.empty())
    {
        UI::drawCenteredText(
            Rectangle{
                0.0f,
                515.0f,
                static_cast<float>(GetScreenWidth()),
                20.0f
            },
            errorMessage,
            20,
            RED
        );
    }
}