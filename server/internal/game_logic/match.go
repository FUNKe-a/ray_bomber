package gamelogic

import (
	"iter"
	"math/rand"
	"net"
	"time"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
)

type MatchPlayer struct {
	spawn    *protocol.Cell
	position *protocol.Cell

	MatchReady bool

	lastRequestID uint64
	nextMoveTick  uint64
}

func (player *MatchPlayer) GetSpawn() *protocol.Cell {
	return proto.Clone(player.spawn).(*protocol.Cell)
}

// 20 values are reserved for tile types
// 0 means empty tile
type Match struct {
	id      uint64
	Players map[net.Conn]*MatchPlayer

	board *protocol.Board
	rules *protocol.RoundRules

	startedAt time.Time
}

func (m *Match) GetId() uint64 {
	return m.id
}

// GetBoard returns a deep copy of the inner match board
func (m *Match) GetBoard() *protocol.Board {
	return proto.Clone(m.board).(*protocol.Board)
}

func (m *Match) GetRules() *protocol.RoundRules {
	return proto.Clone(m.rules).(*protocol.RoundRules)
}

func (m *Match) Start() {
	m.startedAt = time.Now()
}

func (m *Match) GetServerTick() uint64 {
	if m.startedAt.IsZero() {
		return 0
	}

	tickDuration :=
		time.Second / time.Duration(m.rules.GetTickRateHz())

	return uint64(time.Since(m.startedAt) / tickDuration)
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

// Shared by gameplay requests, including bomb placement later.
func (m *Match) AcceptRequest(
	conn net.Conn,
	requestID uint64,
) bool {
	player, exists := m.Players[conn]
	if !exists ||
		requestID == 0 ||
		requestID <= player.lastRequestID {
		return false
	}

	player.lastRequestID = requestID
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
	}

	i := 0

	for conn := range lobbyMembers {
		spawn := spawnPoints[i]

		matchInstance.Players[conn] = &MatchPlayer{
			spawn: &protocol.Cell{
				X: spawn.GetX(),
				Y: spawn.GetY(),
			},
			position: &protocol.Cell{
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
	serverTick uint64,
) (*protocol.Cell, bool) {
	player, exists := m.Players[conn]
	if !exists || serverTick < player.nextMoveTick {
		return nil, false
	}

	pos := &protocol.Cell{
		X: player.position.GetX() + step.GetX(),
		Y: player.position.GetY() + step.GetY(),
	}

	height := int32(m.board.GetHeight())
	width := int32(m.board.GetWidth())

	if pos.GetX() < 0 || pos.GetX() >= width ||
		pos.GetY() < 0 || pos.GetY() >= height {
		return nil, false
	}

	index := int(pos.GetY())*int(width) + int(pos.GetX())

	if index >= len(m.board.Cells) ||
		m.board.Cells[index] != protocol.Terrain_TERRAIN_FLOOR {
		return nil, false
	}

	player.position = pos
	player.nextMoveTick =
		serverTick + uint64(m.rules.GetMovementIntervalTicks())

	return proto.Clone(pos).(*protocol.Cell), true
}

// TODO: implement bomb placement
func (m *Match) PlayerPlaceBomb(lobbyMember net.Conn, newPos *protocol.Cell) {
}
