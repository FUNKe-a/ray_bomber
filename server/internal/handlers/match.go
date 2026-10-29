package handlers

import (
	"errors"
	"maps"
	"net"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

type matchHandler struct {
	Match *gamelogic.Match
}

func rejectAction(
	conn net.Conn,
	roundID uint64,
	requestID uint64,
	reason protocol.RejectionReason,
) error {
	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_ActionRejected{
			ActionRejected: &protocol.ActionRejected{
				RoundId:   roundID,
				RequestId: requestID,
				Reason:    reason,
			},
		},
	}

	return netio.SendMessage(conn, envelope)
}

func (mh *matchHandler) validateAction(
	conn net.Conn,
	roundID uint64,
	requestID uint64,
) protocol.RejectionReason {
	if mh.Match == nil ||
		lobby.Phase != gamelogic.PhasePlaying {
		return protocol.RejectionReason_REJECTION_WRONG_PHASE
	}

	if _, exists := lobby.Members[conn]; !exists {
		return protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	return mh.Match.ValidateAction(conn, roundID, requestID)
}

func (mh *matchHandler) handleMoveRequest(
	conn net.Conn,
	request *protocol.MoveRequest,
) error {
	if request == nil {
		return nil
	}

	reason := mh.validateAction(
		conn,
		request.GetRoundId(),
		request.GetRequestId(),
	)

	if reason != protocol.RejectionReason_REJECTION_UNSPECIFIED {
		return rejectAction(
			conn,
			request.GetRoundId(),
			request.GetRequestId(),
			reason,
		)
	}

	step := &protocol.Cell{}

	switch request.GetDirection() {
	case protocol.MoveRequest_UP:
		step.Y = -1
	case protocol.MoveRequest_RIGHT:
		step.X = 1
	case protocol.MoveRequest_DOWN:
		step.Y = 1
	case protocol.MoveRequest_LEFT:
		step.X = -1
	default:
		return rejectAction(
			conn,
			request.GetRoundId(),
			request.GetRequestId(),
			protocol.RejectionReason_REJECTION_INVALID_REQUEST,
		)
	}

	position, reason := mh.Match.MovePlayer(conn, step)

	if reason != protocol.RejectionReason_REJECTION_UNSPECIFIED {
		return rejectAction(
			conn,
			request.GetRoundId(),
			request.GetRequestId(),
			reason,
		)
	}

	envelope := &protocol.Envelope{
		Payload: &protocol.Envelope_PlayerMovement{
			PlayerMovement: &protocol.PlayerMovement{
				Id:         lobby.Members[conn].GetId(),
				Pos:        position,
				RoundId:    mh.Match.GetId(),
				ServerTick: mh.Match.GetServerTick(),
				RequestId:  request.GetRequestId(),
			},
		},
	}

	return netio.BroadcastMessage(
		maps.Keys(mh.Match.Players),
		envelope,
	)
}

func (mh *matchHandler) handlePlaceBombRequest(
	conn net.Conn,
	request *protocol.PlaceBombRequest,
) error {
	if request == nil {
		return nil
	}

	reason := mh.validateAction(
		conn,
		request.GetRoundId(),
		request.GetRequestId(),
	)

	if reason == protocol.RejectionReason_REJECTION_UNSPECIFIED {
		_, reason = mh.Match.PlaceBomb(
			conn,
			lobby.Members[conn].GetId(),
		)
	}

	if reason != protocol.RejectionReason_REJECTION_UNSPECIFIED {
		return rejectAction(
			conn,
			request.GetRoundId(),
			request.GetRequestId(),
			reason,
		)
	}

	// The next simulation tick publishes the confirmed board.
	return nil
}

func (mh *matchHandler) handleTick() error {
	if mh.Match == nil || lobby.Phase != gamelogic.PhasePlaying {
		return nil
	}

	mh.Match.AdvanceTick()
	step := mh.Match.ResolveRoundStep()

	if step.Result != nil {
		lobby.Phase = gamelogic.PhaseFinished
	}

	var sendErrors []error

	// Publish the final board before eliminations and results.
	if snapshot := mh.Match.TakeWorldSnapshot(); snapshot != nil {
		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_WorldSnapshot{
				WorldSnapshot: snapshot,
			},
		}

		sendErrors = append(
			sendErrors,
			netio.BroadcastMessage(
				maps.Keys(mh.Match.Players),
				envelope,
			),
		)
	}

	for _, elimination := range step.Eliminations {
		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEliminated{
				PlayerEliminated: elimination,
			},
		}

		sendErrors = append(
			sendErrors,
			netio.BroadcastMessage(
				maps.Keys(mh.Match.Players),
				envelope,
			),
		)
	}

	if step.Result != nil {
		envelope := &protocol.Envelope{
			Payload: &protocol.Envelope_MatchEnded{
				MatchEnded: step.Result,
			},
		}

		sendErrors = append(
			sendErrors,
			netio.BroadcastMessage(
				maps.Keys(mh.Match.Players),
				envelope,
			),
		)
	}

	return errors.Join(sendErrors...)
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

func (mh *matchHandler) resetToLobby(
	reason protocol.LobbyResetReason,
) error {
	if mh.Match == nil {
		return nil
	}

	roundID := mh.Match.GetId()

	// Discard bombs, blasts, positions, results, and round readiness.
	mh.Match = nil
	lobby.Phase = gamelogic.PhaseLobby

	for _, member := range lobby.Members {
		member.IsReady = false
	}

	reset := &protocol.Envelope{
		Payload: &protocol.Envelope_LobbyReset{
			LobbyReset: &protocol.LobbyReset{
				RoundId: roundID,
				Reason:  reason,
			},
		},
	}

	resetErr := netio.BroadcastMessage(
		maps.Keys(lobby.Members),
		reset,
	)

	var leaderErr error

	if lobby.LeaderId() != 0 {
		leader := &protocol.Envelope{
			Payload: &protocol.Envelope_PlayerEvent{
				PlayerEvent: &protocol.PlayerEvent{
					Id: lobby.LeaderId(),
					EventType: &protocol.PlayerEvent_SetLeader{
						SetLeader: &protocol.PlayerSetLeader{},
					},
				},
			},
		}

		leaderErr = netio.BroadcastMessage(
			maps.Keys(lobby.Members),
			leader,
		)
	}

	return errors.Join(resetErr, leaderErr)
}

func (mh *matchHandler) cancelPreparation(
	reason protocol.LobbyResetReason,
) error {
	return mh.resetToLobby(reason)
}

func (mh *matchHandler) handleReturnToLobbyRequest(
	conn net.Conn,
	request *protocol.ReturnToLobbyRequest,
) error {
	if request == nil {
		return nil
	}

	if mh.Match == nil ||
		lobby.Phase != gamelogic.PhaseFinished ||
		!mh.Match.IsFinished() {
		return rejectAction(
			conn, request.GetRoundId(), request.GetRequestId(),
			protocol.RejectionReason_REJECTION_WRONG_PHASE,
		)
	}

	reason := mh.Match.ValidateReturnRequest(
		conn,
		request.GetRoundId(),
		request.GetRequestId(),
	)

	if reason != protocol.RejectionReason_REJECTION_UNSPECIFIED {
		return rejectAction(
			conn, request.GetRoundId(), request.GetRequestId(),
			reason,
		)
	}

	member, exists := lobby.Members[conn]

	if !exists {
		return rejectAction(
			conn, request.GetRoundId(), request.GetRequestId(),
			protocol.RejectionReason_REJECTION_INVALID_REQUEST,
		)
	}

	if member.GetId() != lobby.LeaderId() {
		return rejectAction(
			conn, request.GetRoundId(), request.GetRequestId(),
			protocol.RejectionReason_REJECTION_NOT_LEADER,
		)
	}

	return mh.resetToLobby(
		protocol.LobbyResetReason_RESET_ROUND_FINISHED,
	)
}
