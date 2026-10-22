package handlers

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"maps"
	"net"
)

type matchHandler struct {
	Match *gamelogic.Match
}

func (mh *matchHandler) handleMoveRequest(conn net.Conn, msg *protocol.MoveRequest) error {
	if ok := mh.Match.PlayerExists(conn); ok {
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
					Id:  lobby.Members[conn].GetId(),
					Pos: newPos,
				},
			},
		}

		if err := netio.BroadcastMessage(maps.Keys(mh.Match.Players), msg); err != nil {
			return err
		}
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
			StartMatch: &protocol.StartMatch{},
		},
	}

	return netio.BroadcastMessage(maps.Keys(mh.Match.Players), envelope)
}

func (mh *matchHandler) sendMatchSetup() error {
	spawnInfo := make([]*protocol.PlayerSpawnInfo, 0, len(lobby.Members)-1)
	for k, v := range lobby.Members {
		spawnInfo = append(spawnInfo, &protocol.PlayerSpawnInfo{
			PlayerId: v.GetId(),
			Pos: mh.Match.Players[k].GetSpawn(),
		})
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_MatchSetup{
			MatchSetup: &protocol.MatchSetup{
				Spawns: spawnInfo,
				RoundId: mh.Match.GetId(),
				Board: mh.Match.GetBoard(),
				Rules: &protocol.RoundRules{},
			},
		},
	}

	return netio.BroadcastMessage(maps.Keys(mh.Match.Players), envelope)
}
