package handlers

import (
	"errors"
	"maps"
	"net"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

type HandlerFunc func(match *gamelogic.GameMatch, conn net.Conn, payload any) error

func HandleMoveRequest(match *gamelogic.GameMatch, conn net.Conn, msg *protocol.MoveRequest) error {
	player, exists := match.Players[conn]
	if !exists || match.Phase != gamelogic.PhasePlaying {
		return nil
	}

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
	default:
		return nil
	}

	if new_x >= 0 && new_x < int32(len(match.Board[0])) && new_y >= 0 && new_y < int32(len(match.Board)) {
		if match.Board[new_y][new_x] == gamelogic.EmptyTile {
			player.X = new_x
			player.Y = new_y

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
	if match.Phase != gamelogic.PhaseLobby {
		return conn.Close()
	}

	if _, exists := match.Players[conn]; exists {
		// Ignore duplicate join requests from an admitted connection.
		return nil
	}

	if match.IsFull(conn) {
		return conn.Close()
	}

	id, color := match.AddPlayer(conn, msg.Username)

	if id == 0 {
		return conn.Close()
	}

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:    id,
				Color: color,
			},
		},
	}

	if err := netio.SendMessage(conn, response); err != nil {
		return err
	}

	for existingConn, existingPlayer := range match.Players {
		if existingConn == conn {
			continue
		}

		existingPlayerEvent := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEvent{
				PlayerEvent: &protocol.PlayerEvent{
					Id: existingPlayer.ID,
					EventType: &protocol.PlayerEvent_Joined{
						Joined: &protocol.PlayerJoined{
							Username: existingPlayer.Username,
							Color:    existingPlayer.Color,
						},
					},
				},
			},
		}

		if err := netio.SendMessage(conn, existingPlayerEvent); err != nil {
			return err
		}

		if existingPlayer.IsReady {
			existingReadyEvent := &protocol.Envelope{
				Payload: &protocol.Envelope_PlayerEvent{
					PlayerEvent: &protocol.PlayerEvent{
						Id: existingPlayer.ID,
						EventType: &protocol.PlayerEvent_Ready{
							Ready: &protocol.PlayerReady{
								IsReady: true,
							},
						},
					},
				},
			}

			if err := netio.SendMessage(conn, existingReadyEvent); err != nil {
				return err
			}
		}
	}

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
	joinedErr := netio.BroadcastMessage(maps.Keys(match.Players), broadcast)
	leaderErr := broadcastLeader(match)

	return errors.Join(joinedErr, leaderErr)
}

func HandleUpdateReadyState(match *gamelogic.GameMatch, conn net.Conn, msg *protocol.UpdateReadyState) error {
	player, exists := match.Players[conn]
	if !exists || match.Phase != gamelogic.PhaseLobby {
		return nil
	}

	player.IsReady = msg.IsReady

	broadcast := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: player.ID,
				EventType: &protocol.PlayerEvent_Ready{
					Ready: &protocol.PlayerReady{
						IsReady: player.IsReady,
					},
				},
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(match.Players), broadcast)
}

func HandlePlayerDisconnect(match *gamelogic.GameMatch, conn net.Conn) error {
	player, leaderChanged := match.RemovePlayer(conn)
	if player == nil {
		return nil
	}

	left := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: player.ID,
				EventType: &protocol.PlayerEvent_Left{
					Left: &protocol.PlayerLeft{
						Reason: "Player disconnected",
					},
				},
			},
		},
	}

	leftErr := netio.BroadcastMessage(maps.Keys(match.Players), left)

	var leaderErr error
	if leaderChanged {
		leaderErr = broadcastLeader(match)
	}

	// A departing client must not block the remaining ready clients.
	startErr := tryStartMatch(match)

	return errors.Join(leftErr, leaderErr, startErr)
}

func broadcastLeader(match *gamelogic.GameMatch) error {
	if match.LeaderID == 0 {
		return nil
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: match.LeaderID,
				EventType: &protocol.PlayerEvent_SetLeader{
					SetLeader: &protocol.PlayerSetLeader{},
				},
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(match.Players), envelope)
}

func HandleGameStartRequest(
	match *gamelogic.GameMatch,
	conn net.Conn,
	_ *protocol.GameStartRequest,
) error {
	player, exists := match.Players[conn]

	if !exists || match.Phase != gamelogic.PhaseLobby || player.ID != match.LeaderID {
		return nil
	}

	for _, participant := range match.Players {
		if !participant.IsReady {
			return nil
		}
	}

	match.Phase = gamelogic.PhasePreparing

	spawns := make([]*protocol.PlayerSpawnInfo, 0, len(match.Players))

	for _, playerConn := range match.JoinOrder {
		participant := match.Players[playerConn]
		participant.MatchReady = false

		spawns = append(spawns, &protocol.PlayerSpawnInfo{
			PlayerId: participant.ID,
			X:        participant.X,
			Y:        participant.Y,
		})
	}

	gameStart := &protocol.Envelope{
		Payload: &protocol.Envelope_GameStart{
			GameStart: &protocol.GameStart{},
		},
	}

	setup := &protocol.Envelope{
		Payload: &protocol.Envelope_MatchSetup{
			MatchSetup: &protocol.MatchSetup{
				Spawns: spawns,
			},
		},
	}

	// Preserve this order for every client.
	startErr := netio.BroadcastMessage(maps.Keys(match.Players), gameStart)
	setupErr := netio.BroadcastMessage(maps.Keys(match.Players), setup)

	return errors.Join(startErr, setupErr)
}

func HandleClientMatchReady(
	match *gamelogic.GameMatch,
	conn net.Conn,
	_ *protocol.ClientMatchReady,
) error {
	player, exists := match.Players[conn]

	if !exists || match.Phase != gamelogic.PhasePreparing {
		return nil
	}

	player.MatchReady = true
	return tryStartMatch(match)
}

func tryStartMatch(match *gamelogic.GameMatch) error {
	if match.Phase != gamelogic.PhasePreparing ||
		len(match.Players) == 0 {
		return nil
	}

	for _, player := range match.Players {
		if !player.MatchReady {
			return nil
		}
	}

	match.Phase = gamelogic.PhasePlaying

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_StartMatch{
			StartMatch: &protocol.StartMatch{},
		},
	}

	return netio.BroadcastMessage(maps.Keys(match.Players), envelope)
}
