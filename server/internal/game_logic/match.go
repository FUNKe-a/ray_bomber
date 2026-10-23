package gamelogic

import (
	"iter"
	"math/rand"
	"net"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
)

type MatchPlayer struct {
	spawn      *protocol.Cell
	MatchReady bool
}

func (player *MatchPlayer) GetSpawn() *protocol.Cell {
	return player.spawn
}

// 20 values are reserved for tile types
// 0 means empty tile
type Match struct {
	id      uint64
	Players map[net.Conn]*MatchPlayer

	board *protocol.Board
}

func (m *Match) GetId() uint64 {
	return m.id
}

// GetBoard returns a deep copy of the inner match board
func (m *Match) GetBoard() *protocol.Board {
	return proto.Clone(m.board).(*protocol.Board)
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
	}

	i := 0
	for v := range lobbyMembers {
		matchInstance.Players[v] = &MatchPlayer{
			spawn: spawnPoints[i],
		}
		i += 1
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

	return matchInstance
}

func (m *Match) PlayerExists(memberConn net.Conn) bool {
	if _, ok := m.Players[memberConn]; ok {
		return true
	}
	return false
}

func (m *Match) MovePlayer(
	memberConn net.Conn,
	step *protocol.Cell,
) (*protocol.Cell, bool) {
	player, exists := m.Players[memberConn]
	if !exists {
		return nil, false
	}

	pos := &protocol.Cell{
		X: player.spawn.GetX() + step.GetX(),
		Y: player.spawn.GetY() + step.GetY(),
	}

	height := int32(m.board.GetHeight())
	width := int32(m.board.GetWidth())

	if pos.GetX() < 0 || pos.GetX() >= width ||
		pos.GetY() < 0 || pos.GetY() >= height {
		return nil, false
	}

	index := int(pos.GetY())*int(width) + int(pos.GetX())

	if index >= len(m.board.Cells) {
		return nil, false
	}

	if m.board.Cells[index] != protocol.Terrain_TERRAIN_FLOOR {
		return nil, false
	}

	player.spawn = pos
	return pos, true
}

// TODO: implement bomb placement
func (m *Match) PlayerPlaceBomb(lobbyMember net.Conn, newPos *protocol.Cell) {
}
