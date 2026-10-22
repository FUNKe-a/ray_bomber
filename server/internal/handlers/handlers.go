package handlers

import (
	"errors"
	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"maps"
	"net"
)

var lobby = gamelogic.GetLobby()

func HandleMoveRequest(conn net.Conn, msg *protocol.MoveRequest) error {
	player, exists := lobby.LobbyMembers[conn]
	if !exists || lobby.Phase != gamelogic.PhasePlaying {
		return nil
	}

	new_pos := &protocol.Cell{}
	switch msg.Direction {
	case protocol.MoveRequest_UP:
		new_pos.Y -= 1
	case protocol.MoveRequest_RIGHT:
		new_pos.X += 1
	case protocol.MoveRequest_DOWN:
		new_pos.Y += 1
	case protocol.MoveRequest_LEFT:
		new_pos.X -= 1
	default:
		return nil
	}

	msg := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerMovement{
			PlayerMovement: &protocol.PlayerMovement{
				Id: player.Information.GetId(),
				Pos: new_pos,
			},
		},
	}

	if err := netio.BroadcastMessage(maps.Keys(lobby.LobbyMembers), msg); err != nil {
		return err
	}

	return nil
}

func HandleJoinLobbyRequest(conn net.Conn, msg *protocol.JoinLobbyRequest) error {
	if lobby.Phase != gamelogic.PhaseLobby || lobby.IsFull() {
		return conn.Close()
	}

	id, color, err := lobby.AddLobbyMember(conn, msg.Username)
	if err != nil {
		return err
	}

	players := make([]*protocol.PlayerInfo, 0, len(lobby.LobbyMembers)-1)
	for _, p := range lobby.LobbyMembers {
		if p.Information.GetId() != id {
			players = append(players, p.Information)
		}
	}

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:       id,
				LeaderId: lobby.LeaderID,
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
						Username: lobby.LobbyMembers[conn].Information.GetUsername(),
						Color:    color,
					},
				},
			},
		},
	}

	joinedErr := netio.BroadcastMessage(maps.Keys(lobby.LobbyMembers), broadcast)

	return joinedErr
}

func HandleUpdateReadyState(conn net.Conn, msg *protocol.UpdateReadyState) error {
	player := lobby.Players[conn]
	if lobby.Phase != gamelogic.PhaseLobby {
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

	return netio.BroadcastMessage(maps.Keys(lobby.Players), broadcast)
}

func HandlePlayerDisconnect(conn net.Conn) error {
	player, leaderChanged := lobby.RemovePlayer(conn)
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

	leftErr := netio.BroadcastMessage(maps.Keys(lobby.Players), left)

	var leaderErr error
	if leaderChanged && lobby.LeaderID != 0 {
		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEvent{
				PlayerEvent: &protocol.PlayerEvent{
					Id: lobby.LeaderID,
					EventType: &protocol.PlayerEvent_SetLeader{
						SetLeader: &protocol.PlayerSetLeader{},
					},
				}, },
		}

		leaderErr = netio.BroadcastMessage(maps.Keys(lobby.Players), envelope)
	}

	return errors.Join(leftErr, leaderErr)
}

func HandleGameStartRequest(
	conn net.Conn,
) error {
	player := lobby.Players[conn]

	if lobby.Phase != gamelogic.PhaseLobby || player.ID != lobby.LeaderID {
		return nil
	}

	for _, participant := range lobby.Players {
		if !participant.IsReady {
			return nil
		}
	}

	lobby.Phase = gamelogic.PhasePreparing

	spawns := make([]*protocol.PlayerSpawnInfo, 0, len(lobby.Players))
	for _, player := range lobby.Players {
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

	startErr := netio.BroadcastMessage(maps.Keys(lobby.Players), gameStart)
	setupErr := netio.BroadcastMessage(maps.Keys(lobby.Players), setup)

	return errors.Join(startErr, setupErr)
}

func HandleClientMatchReady(
	conn net.Conn,
) error {
	player := lobby.Players[conn]
	if lobby.Phase != gamelogic.PhasePreparing {
		return nil
	}
	player.MatchReady = true

	for _, player := range lobby.Players {
		if !player.MatchReady {
			return nil
		}
	}

	lobby.Phase = gamelogic.PhasePlaying

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_StartMatch{
			StartMatch: &protocol.StartMatch{},
		},
	}

	return netio.BroadcastMessage(maps.Keys(lobby.Players), envelope)
}
