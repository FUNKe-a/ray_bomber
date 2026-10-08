package handlers

import (
	"errors"
	"maps"
	"net"
	"fmt"
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
	if match.Phase != gamelogic.PhaseLobby || match.IsFull() {
		return conn.Close()
	}

	id, color, err := match.AddPlayer(conn, msg.Username)
	if err != nil {
		return err
	}

	players := make([]*protocol.PlayerInfo, 0, len(match.Players) - 1)
	for _, p := range match.Players {
		if p.ID != id {
			info := &protocol.PlayerInfo{
				Id: p.ID,
				Username: p.Username,
				Color: p.Color,
			}
			players = append(players, info)
		}
	}

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:    id,
				Color: color,
				Players: players,
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
	player := match.Players[conn]
	if match.Phase != gamelogic.PhaseLobby {
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

	return errors.Join(leftErr, leaderErr)
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
) error {
	player := match.Players[conn]

	if match.Phase != gamelogic.PhaseLobby || player.ID != match.LeaderID {
		return nil
	}

	for _, participant := range match.Players {
		if !participant.IsReady {
			return nil
		}
	}

	match.Phase = gamelogic.PhasePreparing

	spawns := make([]*protocol.PlayerSpawnInfo, len(match.Players))

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

	startErr := netio.BroadcastMessage(maps.Keys(match.Players), gameStart)
	setupErr := netio.BroadcastMessage(maps.Keys(match.Players), setup)

	return errors.Join(startErr, setupErr)
}

func HandleClientMatchReady(
	match *gamelogic.GameMatch,
	conn net.Conn,
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
