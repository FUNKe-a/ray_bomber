package handlers

import (
	"log/slog"
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
	slog.Debug(
		"MoveRequest received",
		"direction", msg.GetDirection(),
		"round_id", msg.GetRoundId(),
		"request_id", msg.GetRequestId(),
	)

	if !mh.Match.PlayerExists(conn) {
		slog.Debug("Movement ignored: player not found")
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

	if newPos, ok := mh.Match.MovePlayer(conn, step); ok {
		msg := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerMovement{
				PlayerMovement: &protocol.PlayerMovement{
					Id:        lobby.Members[conn].GetId(),
					Pos:       newPos,
					RoundId:   mh.Match.GetId(),
					RequestId: msg.GetRequestId(),
				},
			},
		}

		if err := netio.BroadcastMessage(maps.Keys(mh.Match.Players), msg); err != nil {
			return err
		}
	} else {
		slog.Debug("Movement blocked: destination is outside the board or not floor")
	}

	return nil
}

func (mh *matchHandler) handleClientMatchReady(conn net.Conn) error {
	player := mh.Match.Players[conn]
	if lobby.Phase != gamelogic.PhasePreparing {
		return nil
	}

	player.MatchReady = true

	for _, player := range mh.Match.Players {
		if !player.MatchReady {
			return nil
		}
	}

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
	spawnInfo := make([]*protocol.PlayerSpawnInfo, 0, len(lobby.Members))
	for k, v := range lobby.Members {
		spawnInfo = append(spawnInfo, &protocol.PlayerSpawnInfo{
			PlayerId: v.GetId(),
			Pos:      mh.Match.Players[k].GetSpawn(),
		})
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_MatchSetup{
			MatchSetup: &protocol.MatchSetup{
				Spawns:  spawnInfo,
				RoundId: mh.Match.GetId(),
				Board:   mh.Match.GetBoard(),
				Rules: &protocol.RoundRules{
					TickRateHz:            20,
					BombCapacity:          1,
					BombFuseTicks:         60,
					BlastDurationTicks:    10,
					BlastRange:            2,
					MovementIntervalTicks: 3,
				},
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(mh.Match.Players), envelope)
}
