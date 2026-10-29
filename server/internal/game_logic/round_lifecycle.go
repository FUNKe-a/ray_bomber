package gamelogic

import (
	"net"
	"sort"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
)

type RoundStepResult struct {
	Eliminations []*protocol.PlayerEliminated
	Result       *protocol.MatchEnded
}

func (m *Match) IsFinished() bool {
	return m.result != nil
}

func (m *Match) GetResult() *protocol.MatchEnded {
	if m.result == nil {
		return nil
	}

	return proto.Clone(m.result).(*protocol.MatchEnded)
}

// Removes the connection immediately, but publishes its elimination
// during the next simulation step.
func (m *Match) RemovePlayer(conn net.Conn) {
	player, exists := m.Players[conn]
	if !exists {
		return
	}

	if m.started && !player.Eliminated {
		m.pendingDisconnects = append(
			m.pendingDisconnects,
			&protocol.PlayerEliminated{
				RoundId:  m.id,
				PlayerId: player.ID,
				Reason:   protocol.EliminationReason_ELIMINATION_DISCONNECTED,
			},
		)
	}

	delete(m.Players, conn)
}

// Called after AdvanceTick has completed all world mutations.
func (m *Match) ResolveRoundStep() RoundStepResult {
	if !m.started || m.result != nil {
		return RoundStepResult{}
	}

	eliminations := m.pendingDisconnects
	m.pendingDisconnects = nil

	for _, elimination := range eliminations {
		elimination.ServerTick = m.serverTick
	}

	// First resolve every remaining player's damage.
	for _, player := range m.Players {
		if player.Eliminated {
			continue
		}

		if !player.pendingElimination &&
			!m.IsBlastActive(player.position) {
			continue
		}

		player.Eliminated = true
		player.pendingElimination = false

		eliminations = append(
			eliminations,
			&protocol.PlayerEliminated{
				RoundId:    m.id,
				ServerTick: m.serverTick,
				PlayerId:   player.ID,
				Reason:     protocol.EliminationReason_ELIMINATION_BLAST,
			},
		)
	}

	sort.Slice(eliminations, func(i, j int) bool {
		return eliminations[i].GetPlayerId() <
			eliminations[j].GetPlayerId()
	})

	// Only count survivors after every elimination is applied.
	var survivor *MatchPlayer
	survivorCount := 0

	for _, player := range m.Players {
		if !player.Eliminated {
			survivor = player
			survivorCount++
		}
	}

	if survivorCount > 1 {
		return RoundStepResult{
			Eliminations: eliminations,
		}
	}

	result := &protocol.MatchEnded{
		RoundId:    m.id,
		ServerTick: m.serverTick,
	}

	if survivorCount == 1 {
		winnerID := survivor.ID

		result.Outcome = protocol.RoundOutcome_OUTCOME_WINNER
		result.WinnerId = &winnerID
	} else {
		result.Outcome = protocol.RoundOutcome_OUTCOME_DRAW
	}

	// Latch the outcome before publishing anything.
	m.result = result
	m.started = false

	return RoundStepResult{
		Eliminations: eliminations,
		Result:       m.GetResult(),
	}
}

// Results requests use the same per-player request sequence,
// but do not require the player to be alive.
func (m *Match) ValidateReturnRequest(
	conn net.Conn,
	roundID uint64,
	requestID uint64,
) protocol.RejectionReason {
	if roundID != m.id {
		return protocol.RejectionReason_REJECTION_STALE_ROUND
	}

	player, exists := m.Players[conn]
	if !exists ||
		requestID == 0 ||
		requestID <= player.lastRequestID {
		return protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	player.lastRequestID = requestID

	return protocol.RejectionReason_REJECTION_UNSPECIFIED
}
