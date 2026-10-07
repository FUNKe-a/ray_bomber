#include "ProtobufAdapter.hpp"

#include <stdexcept>

namespace {
    std::optional<PlayerColor> decodeColor(player::Color color)
    {
        switch (color) {
            case player::UNKNOWN: return PlayerColor::Unknown;
            case player::RED:     return PlayerColor::Red;
            case player::GREEN:   return PlayerColor::Green;
            case player::BLUE:    return PlayerColor::Blue;
            case player::YELLOW:  return PlayerColor::Yellow;
            default:             return std::nullopt;
        }
    }

    player::MoveRequest::Direction encodeDirection(Direction direction)
    {
        switch (direction) {
            case Direction::Up:    return player::MoveRequest::UP;
            case Direction::Right: return player::MoveRequest::RIGHT;
            case Direction::Down:  return player::MoveRequest::DOWN;
            case Direction::Left:  return player::MoveRequest::LEFT;
        }

        throw std::invalid_argument("Invalid movement direction.");
    }
}

Envelope ProtobufAdapter::makeJoin(const std::string& name)
{
    Envelope message;
    message.mutable_join_lobby_request()->set_username(name);
    return message;
}

Envelope ProtobufAdapter::makeReady(bool ready)
{
    Envelope message;
    message.mutable_update_ready_state()->set_is_ready(ready);
    return message;
}

Envelope ProtobufAdapter::makeMove(Direction direction)
{
    Envelope message;
    message.mutable_move_request()->set_direction(
        encodeDirection(direction)
    );
    return message;
}

std::optional<GameEvent> ProtobufAdapter::decode(
    const Envelope& message)
{
    switch (message.payload_case()) {
        case Envelope::kJoinLobbyResponse: {
            const auto& response = message.join_lobby_response();
            const auto color = decodeColor(response.color());

            if (!color)
                return std::nullopt;

            return GameEvent{
                LobbyJoined{response.id(), *color}
            };
        }

        case Envelope::kPlayerMovement: {
            const auto& movement = message.player_movement();

            return GameEvent{
                PlayerMoved{
                    movement.id(),
                    movement.x(),
                    movement.y()
                }
            };
        }

        case Envelope::kPlayerEvent: {
            const auto& event = message.player_event();

            switch (event.event_type_case()) {
                case player::PlayerEvent::kJoined: {
                    const auto& joined = event.joined();
                    const auto color = decodeColor(joined.color());

                    if (!color)
                        return std::nullopt;

                    return GameEvent{
                        PlayerJoined{
                            event.id(),
                            joined.username(),
                            *color
                        }
                    };
                }

                case player::PlayerEvent::kLeft:
                    return GameEvent{
                        PlayerLeft{
                            event.id(),
                            event.left().reason()
                        }
                    };

                case player::PlayerEvent::kReady:
                    return GameEvent{
                        PlayerReadyChanged{
                            event.id(),
                            event.ready().is_ready()
                        }
                    };

                case player::PlayerEvent::kSpawned:
                    return GameEvent{
                        PlayerSpawned{
                            event.id(),
                            event.spawned().x(),
                            event.spawned().y()
                        }
                    };

                default:
                    return std::nullopt;
            }
        }

        default:
            return std::nullopt;
    }
}