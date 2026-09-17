#include "Protocol.hpp"

#include <stdexcept>

namespace Protocol
{
    std::vector<std::uint8_t> serialize(const Message& message)
    {
        const std::uint16_t bodyLength =
            static_cast<std::uint16_t>(message.body.size());

        std::vector<std::uint8_t> packet;

        packet.reserve(3 + bodyLength);

        packet.push_back(
            static_cast<std::uint8_t>(message.type)
        );

        packet.push_back(
            static_cast<std::uint8_t>(bodyLength >> 8)
        );

        packet.push_back(
            static_cast<std::uint8_t>(bodyLength & 0xFF)
        );

        packet.insert(
            packet.end(),
            message.body.begin(),
            message.body.end()
        );

        return packet;
    }

    Message createMoveRequest(Direction direction)
    {
        return Message{
            MessageType::MoveRequested,
            {
                static_cast<std::uint8_t>(direction)
            }
        };
    }

    Greeting deserializeGreeting(const Message& message)
    {
        if (message.type != MessageType::Greeting ||
            message.body.size() != 1)
        {
            throw std::runtime_error("Invalid Greeting message");
        }

        return Greeting{
            message.body[0]
        };
    }

    PlayerJoined deserializePlayerJoined(const Message& message)
    {
        if (message.type != MessageType::PlayerJoined ||
            message.body.size() != 3)
        {
            throw std::runtime_error("Invalid PlayerJoined message");
        }

        return PlayerJoined{
            message.body[0],
            message.body[1],
            message.body[2]
        };
    }

    PlayerLeft deserializePlayerLeft(const Message& message)
    {
        if (message.type != MessageType::PlayerLeft ||
            message.body.size() != 1)
        {
            throw std::runtime_error("Invalid PlayerLeft message");
        }

        return PlayerLeft{
            message.body[0]
        };
    }

    PlayerMoved deserializePlayerMoved(const Message& message)
    {
        if (message.type != MessageType::PlayerMoved ||
            message.body.size() != 3)
        {
            throw std::runtime_error("Invalid PlayerMoved message");
        }

        return PlayerMoved{
            message.body[0],
            message.body[1],
            message.body[2]
        };
    }
}