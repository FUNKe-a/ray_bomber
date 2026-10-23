#include "ProtobufAdapter.hpp"

#include <stdexcept>
#include <utility>

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

Envelope ProtobufAdapter::makeMove(
    Direction direction, std::uint64_t roundId, std::uint64_t requestId)
{
    Envelope message;
    message.mutable_move_request()->set_round_id(roundId);
    message.mutable_move_request()->set_request_id(requestId);
    message.mutable_move_request()->set_direction(encodeDirection(direction));
    return message;
}

Envelope ProtobufAdapter::makeGameStart()
{
    Envelope message;
    message.mutable_game_start_request();
    return message;
}

Envelope ProtobufAdapter::makeMatchReady(std::uint64_t roundId)
{
    Envelope message;
    message.mutable_client_match_ready()->set_round_id(roundId);
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
			setup.leader_id = response.leader_id();

			setup.players.reserve(response.players_size());
			for (const auto& player : response.players()) {
            	const auto pcolor = decodeColor(player.color());

            	if (!pcolor)
                	return std::nullopt;

				setup.players.push_back(PlayerInfo{
					player.id(),
					*pcolor,
					player.username(),
                    player.is_ready()
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
                    movement.pos().x(),
                    movement.pos().y(),
                    movement.round_id(),
                    movement.server_tick(),
                    movement.request_id()
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
            const auto& source = message.match_setup();
            if (!source.has_board() || !source.has_rules())
                return std::nullopt;

            const auto& board = source.board();
            if (!Map::validDimensions(board.width(), board.height()) ||
                static_cast<std::uint64_t>(board.cells_size()) !=
                    static_cast<std::uint64_t>(board.width()) * board.height())
                return std::nullopt;

            MatchSetupReceived setup;
            setup.roundId = source.round_id();
            setup.width = board.width();
            setup.height = board.height();
            setup.terrain.reserve(board.cells_size());
            for (const auto terrain : board.cells())
            {
                switch (terrain)
                {
                    case match::TERRAIN_FLOOR: setup.terrain.push_back(Map::Tile::Empty); break;
                    case match::TERRAIN_WALL: setup.terrain.push_back(Map::Tile::Wall); break;
                    case match::TERRAIN_CRATE: setup.terrain.push_back(Map::Tile::Crate); break;
                    default: return std::nullopt;
                }
            }

            const auto& rules = source.rules();
            setup.rules = RoundRules{
                rules.tick_rate_hz(), rules.bomb_capacity(), rules.bomb_fuse_ticks(),
                rules.blast_duration_ticks(), rules.blast_range(), rules.movement_interval_ticks()
            };
            setup.spawns.reserve(source.spawns_size());

            for (const auto& spawn : message.match_setup().spawns())
            {
                setup.spawns.push_back(PlayerSpawn{
                    spawn.player_id(),
                    spawn.pos().x(),
                    spawn.pos().y()
                });
            }

            return GameEvent{std::move(setup)};
        }

        case Envelope::kStartMatch:
            return GameEvent{MatchStarted{
                message.start_match().round_id(),
                message.start_match().server_tick()
            }};

        default:
            return std::nullopt;
    }
}
