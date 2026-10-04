package handlers

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"maps"
	"net"
)

type HandlerFunc func(match *gamelogic.GameMatch, conn net.Conn, payload any) error

func HandleMoveRequest(match *gamelogic.GameMatch, conn net.Conn, msg *protocol.MoveRequest) error {
	player := match.Players[conn]

	new_x := player.X
	new_y := player.Y

	switch msg.Direction {
	case protocol.MoveRequest_UP:
		new_y -= 1
	case protocol.MoveRequest_RIGHT:
		new_x += 1
	case protocol.MoveRequest_DOWN:
		new_y += 1
	case protocol.MoveRequest_LEFT:
		new_x -= 1
	}

	if new_x >= 0 && new_x < int32(len(match.Board[0])) && new_y >= 0 && new_y < int32(len(match.Board)) {
		if match.Board[new_y][new_x] == gamelogic.EmptyTile {
			msg := &protocol.Envelope{
				Payload: &protocol.Envelope_PlayerMovement{
					PlayerMovement: &protocol.PlayerMovement{
						Id: player.ID,
						X:  new_x,
						Y:  new_y,
					},
				},
			}

			if err := netio.BroadcastMessage(maps.Keys(match.Players), msg); err != nil {
				return err
			}
		}
	}

	return nil
}

func HandleJoinLobbyRequest(match *gamelogic.GameMatch, conn net.Conn, msg *protocol.JoinLobbyRequest) error {
	id, color := match.AddPlayer(conn, msg.Username)

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:    id,
				Color: color,
			},
		},
	}
	netio.SendMessage(conn, response)

	broadcast := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: id,
				EventType: &protocol.PlayerEvent_Joined{
					Joined: &protocol.PlayerJoined{
						Username: match.Players[conn].Username,
						Color:    color,
					},
				},
			},
		},
	}
	netio.BroadcastMessage(maps.Keys(match.Players), broadcast)

	return nil
}
