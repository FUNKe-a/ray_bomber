package gamelogic

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
	"iter"
	"math/rand"
	"net"
)

type MatchPlayer struct {
	spawn *protocol.Cell
	MatchReady bool
}

func (player *MatchPlayer) GetSpawn() *protocol.Cell {
	return player.spawn
}

// 20 values are reserved for tile types
// 0 means empty tile
type Match struct {
	id uint64
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
		&protocol.Cell{X: 1, Y: 1},
		&protocol.Cell{X: 13, Y: 1},
		&protocol.Cell{X: 1, Y: 11},
		&protocol.Cell{X: 13, Y: 11},
	}
	rand.Shuffle(len(spawnPoints), func(i, j int) {
		spawnPoints[i], spawnPoints[j] = spawnPoints[j], spawnPoints[i]
	})

	matchInstance := &Match{}

	i := 0
	for v := range lobbyMembers {
		matchInstance.Players[v] = &MatchPlayer{
			spawn: spawnPoints[i],
		}
		i += 1
	}

	matchInstance.board.Height = 13
	matchInstance.board.Width = 15
	matchInstance.board.Cells = []protocol.Terrain {
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
		2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
		2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2,
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

func (m *Match) MovePlayer(memberConn net.Conn, step *protocol.Cell) (*protocol.Cell, bool) {
	pos := &protocol.Cell {
		X: m.Players[memberConn].spawn.GetX() + step.GetX(),
		Y: m.Players[memberConn].spawn.GetY() + step.GetY(),
	}
	height := int32(m.board.GetHeight())
	width := int32(m.board.GetWidth())

	if pos.GetX() >= 0 && pos.GetX() < height && pos.GetY() >= 0 && pos.GetY() < width {
		if m.board.Cells[pos.GetY() * width + pos.GetX()] == protocol.Terrain_TERRAIN_FLOOR {
			player := m.Players[memberConn]
			player.spawn = pos
			return pos, true
		}
	}
	return nil, false
}

// TODO: implement bomb placement
func (m *Match) PlayerPlaceBomb(lobbyMember net.Conn, newPos *protocol.Cell) {
}
