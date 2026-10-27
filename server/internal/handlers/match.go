package handlers

import (
	"maps"
	"net"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

type matchHandler struct {
	Match *gamelogic.Match
}

func (mh *matchHandler) handleMoveRequest(conn net.Conn, msg *protocol.MoveRequest) error {
	if mh.Match == nil || msg == nil ||
		lobby.Phase != gamelogic.PhasePlaying {
		return nil
	}

	member, exists := lobby.Members[conn]
	if !exists || !mh.Match.PlayerExists(conn) {
		return nil
	}

	if msg.GetRoundId() != mh.Match.GetId() {
		return nil
	}

	if !mh.Match.AcceptRequest(conn, msg.GetRequestId()) {
		return nil
	}

	step := &protocol.Cell{}
	switch msg.Direction {
	case protocol.MoveRequest_UP:
		step.Y -= 1
	case protocol.MoveRequest_RIGHT:
		step.X += 1
	case protocol.MoveRequest_DOWN:
		step.Y += 1
	case protocol.MoveRequest_LEFT:
		step.X -= 1
	default:
		return nil
	}

	serverTick := mh.Match.GetServerTick()

	position, moved := mh.Match.MovePlayer(
		conn,
		step,
		serverTick,
	)

	if !moved {
		return nil
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerMovement{
			PlayerMovement: &protocol.PlayerMovement{
				Id:         member.GetId(),
				Pos:        position,
				RoundId:    mh.Match.GetId(),
				ServerTick: serverTick,
				RequestId:  msg.GetRequestId(),
			},
		},
	}

	return netio.BroadcastMessage(
		maps.Keys(mh.Match.Players),
		envelope,
	)
}

func (mh *matchHandler) handleClientMatchReady(conn net.Conn, msg *protocol.ClientMatchReady) error {
	if mh.Match == nil || msg == nil ||
		lobby.Phase != gamelogic.PhasePreparing {
		return nil
	}

	if !mh.Match.SetPlayerReady(conn, msg.GetRoundId()) {
		return nil
	}

	if !mh.Match.AllPlayersReady() {
		return nil
	}

	mh.Match.Start()
	lobby.Phase = gamelogic.PhasePlaying

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_StartMatch{
			StartMatch: &protocol.StartMatch{
				RoundId:    mh.Match.GetId(),
				ServerTick: 0,
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(mh.Match.Players), envelope)
}

func (mh *matchHandler) sendMatchSetup() error {
	spawns := make([]*protocol.PlayerSpawnInfo, 0, len(mh.Match.Players))
	for conn, participant := range mh.Match.Players {
		spawns = append(spawns, &protocol.PlayerSpawnInfo{
			PlayerId: lobby.Members[conn].GetId(),
			Pos:      participant.GetSpawn(),
		})
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_MatchSetup{
			MatchSetup: &protocol.MatchSetup{
				RoundId: mh.Match.GetId(),
				Board:   mh.Match.GetBoard(),
				Rules:   mh.Match.GetRules(),
				Spawns:  spawns,
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(mh.Match.Players), envelope)
}

// Call after a board-changing simulation tick, once bomb logic exists.
// Player movement alone does not require a board snapshot.
func (mh *matchHandler) sendWorldSnapshot(
	serverTick uint64,
) error {
	if mh.Match == nil ||
		lobby.Phase != gamelogic.PhasePlaying {
		return nil
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_WorldSnapshot{
			WorldSnapshot: &protocol.WorldSnapshot{
				RoundId:    mh.Match.GetId(),
				ServerTick: serverTick,
				Board:      mh.Match.GetBoard(),
			},
		},
	}

	return netio.BroadcastMessage(
		maps.Keys(mh.Match.Players),
		envelope,
	)
}

func (mh *matchHandler) cancelPreparation(
	reason protocol.LobbyResetReason,
) error {
	if mh.Match == nil {
		return nil
	}

	roundID := mh.Match.GetId()

	mh.Match = nil
	lobby.Phase = gamelogic.PhaseLobby

	for _, member := range lobby.Members {
		member.IsReady = false
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_LobbyReset{
			LobbyReset: &protocol.LobbyReset{
				RoundId: roundID,
				Reason:  reason,
			},
		},
	}

	return netio.BroadcastMessage(
		maps.Keys(lobby.Members),
		envelope,
	)
}
