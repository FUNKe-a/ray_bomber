package handlers

import (
	"errors"
	"maps"
	"net"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

func handleJoinLobbyRequest(conn net.Conn, msg *protocol.JoinLobbyRequest) error {
	if lobby.Phase != gamelogic.PhaseLobby || lobby.IsFull() {
		return conn.Close()
	}

	id, color, err := lobby.AddMember(conn, msg.Username)
	if err != nil {
		return err
	}

	players := make([]*protocol.PlayerInfo, 0, len(lobby.Members)-1)
	for _, p := range lobby.Members {
		if p.GetId() != id {
			players = append(players, p)
		}
	}

	response := &protocol.Envelope{
		Payload: &protocol.Envelope_JoinLobbyResponse{
			JoinLobbyResponse: &protocol.JoinLobbyResponse{
				Id:       id,
				LeaderId: lobby.LeaderId(),
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
						Username: lobby.Members[conn].GetUsername(),
						Color:    color,
					},
				},
			},
		},
	}

	joinedErr := netio.BroadcastMessage(maps.Keys(lobby.Members), broadcast)

	return joinedErr
}

func handleUpdateReadyState(conn net.Conn, msg *protocol.UpdateReadyState) error {
	player := lobby.Members[conn]
	if lobby.Phase != gamelogic.PhaseLobby {
		return nil
	}

	player.IsReady = msg.IsReady

	broadcast := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: player.GetId(),
				EventType: &protocol.PlayerEvent_Ready{
					Ready: &protocol.PlayerReady{
						IsReady: player.GetIsReady(),
					},
				},
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(lobby.Members), broadcast)
}

func handlePlayerDisconnect(conn net.Conn) error {
	player, leaderChanged := lobby.RemoveMember(conn)
	if player == nil {
		return nil
	}

	left := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerEvent{
			PlayerEvent: &protocol.PlayerEvent{
				Id: player.GetId(),
				EventType: &protocol.PlayerEvent_Left{
					Left: &protocol.PlayerLeft{
						Reason: "Player disconnected",
					},
				},
			},
		},
	}

	leftErr := netio.BroadcastMessage(maps.Keys(lobby.Members), left)

	var leaderErr error
	if leaderChanged && lobby.LeaderId() != 0 {
		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEvent{
				PlayerEvent: &protocol.PlayerEvent{
					Id: lobby.LeaderId(),
					EventType: &protocol.PlayerEvent_SetLeader{
						SetLeader: &protocol.PlayerSetLeader{},
					},
				}},
		}

		leaderErr = netio.BroadcastMessage(maps.Keys(lobby.Members), envelope)
	}

	return errors.Join(leftErr, leaderErr)
}

func handleGameStartRequest(
	conn net.Conn,
) (*gamelogic.Match, error) {
	player, exists := lobby.Members[conn]

	if !exists || lobby.Phase != gamelogic.PhaseLobby || player.GetId() != lobby.LeaderId() {
		return nil, nil
	}

	for _, participant := range lobby.Members {
		if !participant.GetIsReady() {
			return nil, nil
		}
	}

	lobby.Phase = gamelogic.PhasePreparing
	match := lobby.StartMatch()

	gameStart := &protocol.Envelope{
		Payload: &protocol.Envelope_GameStart{
			GameStart: &protocol.GameStart{
				RoundId: match.GetId(),
			},
		},
	}

	startErr := netio.BroadcastMessage(maps.Keys(lobby.Members), gameStart)

	return match, startErr
}
