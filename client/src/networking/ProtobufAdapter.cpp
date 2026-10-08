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

    match::MoveRequest::Direction encodeDirection(Direction direction)
    {
        switch (direction) {
            case Direction::Up:    return match::MoveRequest::UP;
            case Direction::Right: return match::MoveRequest::RIGHT;
            case Direction::Down:  return match::MoveRequest::DOWN;
            case Direction::Left:  return match::MoveRequest::LEFT;
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

Envelope ProtobufAdapter::makeGameStart()
{
    Envelope message;
    message.mutable_game_start_request();
    return message;
}

Envelope ProtobufAdapter::makeMatchReady()
{
    Envelope message;
    message.mutable_client_match_ready();
    return message;
}

std::optional<GameEvent> ProtobufAdapter::decode(
    const Envelope& message)
{
    switch (message.payload_case()) {
        case Envelope::kJoinLobbyResponse: {
			LobbyJoined setup;
            const auto& response = message.join_lobby_response();
            const auto color = decodeColor(response.color());

            if (!color)
                return std::nullopt;

			setup.id = response.id();
			setup.color = *color;

			setup.players.reserve(response.players_size());
			for (const auto& player : response.players()) {
            	const auto pcolor = decodeColor(player.color());

            	if (!pcolor)
                	return std::nullopt;

				setup.players.push_back(PlayerInfo{
					player.id(),
					*pcolor,
					player.username(),
				});
			}

            return GameEvent{
				setup
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
                case lobby::PlayerEvent::kJoined: {
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

                case lobby::PlayerEvent::kSetLeader:
                    return GameEvent{
                        LobbyLeaderChanged{event.id()}
                    };

                case lobby::PlayerEvent::kLeft:
                    return GameEvent{
                        PlayerLeft{
                            event.id(),
                            event.left().reason()
                        }
                    };

                case lobby::PlayerEvent::kReady:
                    return GameEvent{
                        PlayerReadyChanged{
                            event.id(),
                            event.ready().is_ready()
                        }
                    };

                default:
                    return std::nullopt;
            }
        }

        case Envelope::kGameStart:
            return GameEvent{MatchPreparationStarted{}};

        case Envelope::kMatchSetup:
        {
            MatchSetupReceived setup;
            setup.spawns.reserve(message.match_setup().spawns_size());

            for (const auto& spawn : message.match_setup().spawns())
            {
                setup.spawns.push_back(PlayerSpawn{
                    spawn.player_id(),
                    spawn.x(),
                    spawn.y()
                });
            }

            return GameEvent{std::move(setup)};
        }

        case Envelope::kStartMatch:
            return GameEvent{MatchStarted{}};

        default:
            return std::nullopt;
    }
}
