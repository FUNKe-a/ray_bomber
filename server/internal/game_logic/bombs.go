package gamelogic

import (
	"net"
	"slices"
	"sort"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
)

type Bomb struct {
	ID            uint64
	OwnerID       uint32
	Position      Position
	BlastRange    uint32
	ExplodeAtTick uint64

	owner    *MatchPlayer
	strategy ExplosionStrategy
}

func (m *Match) PlaceBomb(
	conn net.Conn,
	ownerID uint32,
) (uint64, protocol.RejectionReason) {
	player, exists := m.Players[conn]
	if !exists {
		return 0,
			protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	if player.Eliminated {
		return 0,
			protocol.RejectionReason_REJECTION_PLAYER_ELIMINATED
	}

	if player.activeBombs >= m.rules.GetBombCapacity() {
		return 0,
			protocol.RejectionReason_REJECTION_BOMB_CAPACITY_REACHED
	}

	if _, occupied := m.bombAt[player.position]; occupied {
		return 0,
			protocol.RejectionReason_REJECTION_TILE_BLOCKED
	}

	index := int(player.position.Y)*int(m.board.GetWidth()) +
		int(player.position.X)

	if m.board.Cells[index] != protocol.Terrain_TERRAIN_FLOOR {
		return 0,
			protocol.RejectionReason_REJECTION_TILE_BLOCKED
	}

	if m.nextBombID == 0 {
		return 0,
			protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	id := m.nextBombID
	m.nextBombID++

	explodeAt := m.serverTick +
		uint64(m.rules.GetBombFuseTicks())

	// A bomb placed in an active blast detonates on the next step.
	if m.IsBlastActive(player.position) {
		explodeAt = m.serverTick
	}

	bomb := &Bomb{
		ID:            id,
		OwnerID:       ownerID,
		Position:      player.position,
		BlastRange:    m.rules.GetBlastRange(),
		ExplodeAtTick: explodeAt,
		owner:         player,
		strategy:      m.explosionStrategy,
	}

	m.bombs[id] = bomb
	m.bombAt[bomb.Position] = id
	player.activeBombs++

	return id, protocol.RejectionReason_REJECTION_UNSPECIFIED
}

func (m *Match) IsBlastActive(position Position) bool {
	expiresAt, exists := m.blasts[position]
	return exists && expiresAt > m.serverTick
}

func (m *Match) AdvanceTick() {
	if !m.started {
		return
	}

	m.serverTick++

	for position, expiresAt := range m.blasts {
		if expiresAt <= m.serverTick {
			delete(m.blasts, position)
		}
	}

	var queue []uint64
	scheduled := make(map[uint64]bool)

	for id, bomb := range m.bombs {
		if bomb.ExplodeAtTick <= m.serverTick {
			queue = append(queue, id)
			scheduled[id] = true
		}
	}

	// Go map iteration order must not determine explosion ordering.
	sort.Slice(queue, func(i, j int) bool {
		return queue[i] < queue[j]
	})

	// Freeze bomb positions for this tick's propagation calculations.
	bombPositions := make(map[Position]uint64, len(m.bombAt))

	for position, id := range m.bombAt {
		bombPositions[position] = id
	}

	view := explosionView{
		board: m.board,
		bombs: bombPositions,
	}

	affected := make(map[Position]bool)

	for len(queue) > 0 {
		id := queue[0]
		queue = queue[1:]

		bomb, exists := m.bombs[id]
		if !exists {
			continue
		}

		// Remove before processing the blast to prevent double detonation.
		delete(m.bombs, id)
		delete(m.bombAt, bomb.Position)
		bomb.owner.activeBombs--

		cells := bomb.strategy.Calculate(
			view,
			bomb.Position,
			bomb.BlastRange,
		)

		for _, position := range cells {
			affected[position] = true

			otherID, hasBomb := bombPositions[position]

			if hasBomb && !scheduled[otherID] {
				scheduled[otherID] = true
				queue = append(queue, otherID)
			}
		}
	}

	// Terrain is changed only after all blast areas are calculated.
	expiresAt := m.serverTick +
		uint64(m.rules.GetBlastDurationTicks())

	for position := range affected {
		index := int(position.Y)*int(m.board.GetWidth()) +
			int(position.X)

		if m.board.Cells[index] ==
			protocol.Terrain_TERRAIN_CRATE {
			m.board.Cells[index] =
				protocol.Terrain_TERRAIN_FLOOR
		}

		// Overlapping blasts keep the latest expiration.
		if m.blasts[position] < expiresAt {
			m.blasts[position] = expiresAt
		}
	}
}

// Called once after a simulation tick.
// Returns nil when the visible board has not changed.
func (m *Match) TakeWorldSnapshot() *protocol.WorldSnapshot {
	board := m.GetBoard()

	if slices.Equal(board.Cells, m.lastSnapshot) {
		return nil
	}

	m.lastSnapshot = slices.Clone(board.Cells)

	return &protocol.WorldSnapshot{
		RoundId:    m.id,
		ServerTick: m.serverTick,
		Board:      board,
	}
}
