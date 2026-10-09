package handlers

import (
	"errors"
	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"maps"
	"net"
)

var gameInfo = gamelogic.GetGameInfo()

func HandleMoveRequest(conn net.Conn, msg *protocol.MoveRequest) error {
	player, exists := gameInfo.Players[conn]
	if !exists || gameInfo.Phase != gamelogic.PhasePlaying {
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

	if new_x >= 0 && new_x < int32(len(gameInfo.Board[0])) && new_y >= 0 && new_y < int32(len(gameInfo.Board)) {
		if gameInfo.Board[new_y][new_x] == gamelogic.EmptyTile {
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

			if err := netio.BroadcastMessage(maps.Keys(gameInfo.Players), msg); err != nil {
				return err
			}
		}
	}

	return nil
}

func HandleJoinLobbyRequest(conn net.Conn, msg *protocol.JoinLobbyRequest) error {
	if gameInfo.Phase != gamelogic.PhaseLobby || gameInfo.IsFull() {
		return conn.Close()
	}

	id, color, err := gameInfo.AddPlayer(conn, msg.Username)
	if err != nil {
		return err
	}

	players := make([]*protocol.PlayerInfo, 0, len(gameInfo.Players)-1)
	for _, p := range gameInfo.Players {
		if p.ID != id {
			info := &protocol.PlayerInfo{
				Id:       p.ID,
				Username: p.Username,
				Color:    p.Color,
			}
			players = append(players, info)
		}
	}

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:       id,
				LeaderId: gameInfo.LeaderID,
				Color:    color,
				Players:  players,
			},
		},
	}

	if err := netio.SendMessage(conn, response); err != nil {
		return err
	}

	broadcast := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: id,
				EventType: &protocol.PlayerEvent_Joined{
					Joined: &protocol.PlayerJoined{
						Username: gameInfo.Players[conn].Username,
						Color:    color,
					},
				},
			},
		},
	}

	joinedErr := netio.BroadcastMessage(maps.Keys(gameInfo.Players), broadcast)

	return joinedErr
}

func HandleUpdateReadyState(conn net.Conn, msg *protocol.UpdateReadyState) error {
	player := gameInfo.Players[conn]
	if gameInfo.Phase != gamelogic.PhaseLobby {
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

	return netio.BroadcastMessage(maps.Keys(gameInfo.Players), broadcast)
}

func HandlePlayerDisconnect(conn net.Conn) error {
	player, leaderChanged := gameInfo.RemovePlayer(conn)

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

	leftErr := netio.BroadcastMessage(maps.Keys(gameInfo.Players), left)

	var leaderErr error
	if leaderChanged && gameInfo.LeaderID != 0 {

		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEvent{
				PlayerEvent: &protocol.PlayerEvent{
					Id: gameInfo.LeaderID,
					EventType: &protocol.PlayerEvent_SetLeader{
						SetLeader: &protocol.PlayerSetLeader{},
					},
				},
			},
		}

		netio.BroadcastMessage(maps.Keys(gameInfo.Players), envelope)
	}

	return errors.Join(leftErr, leaderErr)
}

func HandleGameStartRequest(
	conn net.Conn,
) error {
	player := gameInfo.Players[conn]

	if gameInfo.Phase != gamelogic.PhaseLobby || player.ID != gameInfo.LeaderID {
		return nil
	}

	for _, participant := range gameInfo.Players {
		if !participant.IsReady {
			return nil
		}
	}

	gameInfo.Phase = gamelogic.PhasePreparing

	spawns := make([]*protocol.PlayerSpawnInfo, 0, len(gameInfo.Players))
	for _, player := range gameInfo.Players {
		spawns = append(spawns, &protocol.PlayerSpawnInfo{
			PlayerId: player.ID,
			X:        player.X,
			Y:        player.Y,
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

	startErr := netio.BroadcastMessage(maps.Keys(gameInfo.Players), gameStart)
	setupErr := netio.BroadcastMessage(maps.Keys(gameInfo.Players), setup)

	return errors.Join(startErr, setupErr)
}

func HandleClientMatchReady(
	conn net.Conn,
) error {
	player := gameInfo.Players[conn]
	if gameInfo.Phase != gamelogic.PhasePreparing {
		return nil
	}
	player.MatchReady = true

	for _, player := range gameInfo.Players {
		if !player.MatchReady {
			return nil
		}
	}

	gameInfo.Phase = gamelogic.PhasePlaying

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_StartMatch{
			StartMatch: &protocol.StartMatch{},
		},
	}

	return netio.BroadcastMessage(maps.Keys(gameInfo.Players), envelope)
}
