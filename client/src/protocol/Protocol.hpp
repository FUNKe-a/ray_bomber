#pragma once

#include <cstdint>
#include <vector>

namespace Protocol
{
    enum class MessageType : std::uint8_t
    {
        Greeting = 0,
        PlayerJoined = 1,
        PlayerLeft = 2,
        MoveRequested = 3,
        PlayerMoved = 4
    };

    enum class Direction : std::uint8_t
    {
        Up = 0,
        Down = 1,
        Left = 2,
        Right = 3
    };

    struct Message
    {
        MessageType type;
        std::vector<std::uint8_t> body;
    };

    struct Greeting
    {
        std::uint8_t id;
    };

    struct PlayerJoined
    {
        std::uint8_t id;
        std::uint8_t startX;
        std::uint8_t startY;
    };

    struct PlayerLeft
    {
        std::uint8_t id;
    };

    struct MoveRequested
    {
        Direction direction;
    };

    struct PlayerMoved
    {
        std::uint8_t id;
        std::uint8_t x;
        std::uint8_t y;
    };

    std::vector<std::uint8_t> serialize(const Message& message);

    Message createMoveRequest(Direction direction);

    Greeting deserializeGreeting(const Message& message);

    PlayerJoined deserializePlayerJoined(const Message& message);

    PlayerLeft deserializePlayerLeft(const Message& message);

    PlayerMoved deserializePlayerMoved(const Message& message);
}