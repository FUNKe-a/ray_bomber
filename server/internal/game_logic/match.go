package gamelogic

import (
	"iter"
	"math/rand"
	"net"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
)

type MatchPlayer struct {
	ID       uint32
	spawn    *protocol.Cell
	position Position

	MatchReady         bool
	Eliminated         bool
	pendingElimination bool

	activeBombs   uint32
	lastRequestID uint64
	nextMoveTick  uint64
}

func (player *MatchPlayer) GetSpawn() *protocol.Cell {
	return &protocol.Cell{
		X: player.spawn.GetX(),
		Y: player.spawn.GetY(),
	}
}

type Match struct {
	id      uint64
	Players map[net.Conn]*MatchPlayer

	// This board contains only floor, walls, and crates.
	board  *protocol.Board
	rules  *protocol.RoundRules
	result *protocol.MatchEnded

	started    bool
	serverTick uint64

	bombs      map[uint64]*Bomb
	bombAt     map[Position]uint64
	blasts     map[Position]uint64
	nextBombID uint64

	explosionStrategy  ExplosionStrategy
	lastSnapshot       []protocol.Terrain
	pendingDisconnects []*protocol.PlayerEliminated
}

func (m *Match) GetId() uint64 {
	return m.id
}

func (m *Match) GetBoard() *protocol.Board {
	board := proto.Clone(m.board).(*protocol.Board)
	width := int(board.GetWidth())

	for position := range m.bombAt {
		index := int(position.Y)*width + int(position.X)
		board.Cells[index] = protocol.Terrain_TERRAIN_BOMB
	}

	// Explosions take visual priority over bombs.
	for position, expiresAt := range m.blasts {
		if expiresAt <= m.serverTick {
			continue
		}

		index := int(position.Y)*width + int(position.X)
		board.Cells[index] = protocol.Terrain_TERRAIN_EXPLOSION
	}

	return board
}

func (m *Match) GetRules() *protocol.RoundRules {
	return proto.Clone(m.rules).(*protocol.RoundRules)
}

func (m *Match) Start() {
	m.started = true
	m.serverTick = 0
}

func (m *Match) GetServerTick() uint64 {
	return m.serverTick
}

func (m *Match) SetPlayerReady(
	conn net.Conn,
	roundID uint64,
) bool {
	if roundID != m.id {
		return false
	}

	player, exists := m.Players[conn]
	if !exists {
		return false
	}

	player.MatchReady = true
	return true
}

func (m *Match) AllPlayersReady() bool {
	if len(m.Players) == 0 {
		return false
	}

	for _, player := range m.Players {
		if !player.MatchReady {
			return false
		}
	}

	return true
}

func NewMatch(lobbyMembers iter.Seq[net.Conn]) *Match {
	spawnPoints := []*protocol.Cell{
		{X: 1, Y: 1},
		{X: 13, Y: 1},
		{X: 1, Y: 11},
		{X: 13, Y: 11},
	}
	rand.Shuffle(len(spawnPoints), func(i, j int) {
		spawnPoints[i], spawnPoints[j] = spawnPoints[j], spawnPoints[i]
	})

	matchInstance := &Match{
		Players: make(map[net.Conn]*MatchPlayer),
		board:   &protocol.Board{},

		rules: &protocol.RoundRules{
			TickRateHz:            20,
			BombCapacity:          1,
			BombFuseTicks:         60,
			BlastDurationTicks:    10,
			BlastRange:            2,
			MovementIntervalTicks: 3,
		},

		bombs:      make(map[uint64]*Bomb),
		bombAt:     make(map[Position]uint64),
		blasts:     make(map[Position]uint64),
		nextBombID: 1,

		explosionStrategy: CrossBlastStrategy{},
	}

	i := 0

	for conn := range lobbyMembers {
		spawn := spawnPoints[i]

		matchInstance.Players[conn] = &MatchPlayer{
			spawn: &protocol.Cell{
				X: spawn.GetX(),
				Y: spawn.GetY(),
			},
			position: Position{
				X: spawn.GetX(),
				Y: spawn.GetY(),
			},
		}

		i++
	}

	matchInstance.board.Height = 13
	matchInstance.board.Width = 15
	matchInstance.board.Cells = []protocol.Terrain{
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	}

	crateCells := []protocol.Cell{
		{X: 3, Y: 3},
		{X: 7, Y: 3},
		{X: 11, Y: 3},
		{X: 3, Y: 5},
		{X: 7, Y: 5},
		{X: 11, Y: 5},
		{X: 3, Y: 7},
		{X: 7, Y: 7},
		{X: 11, Y: 7},
		{X: 3, Y: 9},
		{X: 7, Y: 9},
		{X: 11, Y: 9},
	}

	for _, cell := range crateCells {
		index := int(cell.GetY())*
			int(matchInstance.board.GetWidth()) +
			int(cell.GetX())

		if matchInstance.board.Cells[index] ==
			protocol.Terrain_TERRAIN_FLOOR {
			matchInstance.board.Cells[index] =
				protocol.Terrain_TERRAIN_CRATE
		}
	}

	matchInstance.lastSnapshot = append(
		[]protocol.Terrain(nil),
		matchInstance.board.Cells...,
	)

	return matchInstance
}

func (m *Match) PlayerExists(memberConn net.Conn) bool {
	if _, ok := m.Players[memberConn]; ok {
		return true
	}
	return false
}

func (m *Match) MovePlayer(
	conn net.Conn,
	step *protocol.Cell,
) (*protocol.Cell, protocol.RejectionReason) {
	player, exists := m.Players[conn]
	if !exists {
		return nil,
			protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	if m.serverTick < player.nextMoveTick {
		return nil,
			protocol.RejectionReason_REJECTION_MOVE_COOLDOWN
	}

	position := Position{
		X: player.position.X + step.GetX(),
		Y: player.position.Y + step.GetY(),
	}

	width := int32(m.board.GetWidth())
	height := int32(m.board.GetHeight())

	if position.X < 0 || position.X >= width ||
		position.Y < 0 || position.Y >= height {
		return nil,
			protocol.RejectionReason_REJECTION_TILE_BLOCKED
	}

	index := int(position.Y)*int(width) + int(position.X)

	if m.board.Cells[index] != protocol.Terrain_TERRAIN_FLOOR {
		return nil,
			protocol.RejectionReason_REJECTION_TILE_BLOCKED
	}

	if _, occupied := m.bombAt[position]; occupied {
		return nil,
			protocol.RejectionReason_REJECTION_TILE_BLOCKED
	}

	player.position = position
	if m.IsBlastActive(position) {
		player.pendingElimination = true
	}

	player.nextMoveTick =
		m.serverTick + uint64(m.rules.GetMovementIntervalTicks())

	return &protocol.Cell{
		X: position.X,
		Y: position.Y,
	}, protocol.RejectionReason_REJECTION_UNSPECIFIED
}

func (m *Match) ValidateAction(
	conn net.Conn,
	roundID uint64,
	requestID uint64,
) protocol.RejectionReason {
	if !m.started {
		return protocol.RejectionReason_REJECTION_WRONG_PHASE
	}

	if roundID != m.id {
		return protocol.RejectionReason_REJECTION_STALE_ROUND
	}

	player, exists := m.Players[conn]
	if !exists {
		return protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	if requestID == 0 || requestID <= player.lastRequestID {
		return protocol.RejectionReason_REJECTION_INVALID_REQUEST
	}

	// A rejected gameplay attempt also consumes its request ID.
	player.lastRequestID = requestID

	if player.Eliminated || player.pendingElimination {
		return protocol.RejectionReason_REJECTION_PLAYER_ELIMINATED
	}

	return protocol.RejectionReason_REJECTION_UNSPECIFIED
}
