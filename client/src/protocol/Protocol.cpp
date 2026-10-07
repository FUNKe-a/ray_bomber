#include "Protocol.hpp"

#include <stdexcept>
#include <utility>

Envelope Protocol::createJoinLobbyRequest(const std::string& name)
{
    Envelope message;
    message.mutable_join_lobby_request()->set_username(name);
    return message;
}

Envelope Protocol::createReadyRequest(bool ready)
{
    Envelope message;
    message.mutable_update_ready_state()->set_is_ready(ready);
    return message;
}

Envelope Protocol::createMoveRequest(player::MoveRequest::Direction direction)
{
    if (direction != player::MoveRequest::UP &&
        direction != player::MoveRequest::RIGHT &&
        direction != player::MoveRequest::DOWN &&
        direction != player::MoveRequest::LEFT)
        throw std::invalid_argument("Invalid movement direction.");

    Envelope message;
    message.mutable_move_request()->set_direction(direction);
    return message;
}

std::vector<std::uint8_t> Protocol::serialize(const Envelope& message)
{
    if (message.payload_case() == Envelope::PAYLOAD_NOT_SET)
        throw std::invalid_argument("Envelope has no payload.");
    if (message.ByteSizeLong() > MaxBodySize)
        throw std::runtime_error("Envelope exceeds frame limit.");

    std::string body;
    if (!message.SerializeToString(&body))
        throw std::runtime_error("Could not serialize envelope.");
    return frameBody(body);
}

bool Protocol::deserialize(
    std::span<const std::uint8_t> body, Envelope& message)
{
    if (body.empty() || body.size() > MaxBodySize)
        return false;
    Envelope parsed;
    if (!parsed.ParseFromArray(body.data(), static_cast<int>(body.size())) ||
        parsed.payload_case() == Envelope::PAYLOAD_NOT_SET)
        return false;
    message = std::move(parsed);
    return true;
}

bool Protocol::isServerMessage(const Envelope& message)
{
    switch (message.payload_case())
    {
        case Envelope::kJoinLobbyResponse:
            return player::Color_IsValid(message.join_lobby_response().color());
        case Envelope::kPlayerEvent:
            switch (message.player_event().event_type_case())
            {
                case player::PlayerEvent::kJoined:
                    return player::Color_IsValid(
                        message.player_event().joined().color());
                case player::PlayerEvent::kLeft:
                case player::PlayerEvent::kReady:
                    return true;
                default:
                    return false;
            }
        case Envelope::kPlayerMovement:
            return true;
        default:
            return false;
    }
}
