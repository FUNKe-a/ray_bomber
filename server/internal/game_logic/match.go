package gamelogic

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"iter"
	"math/rand"
)

type matchPlayer struct {
	Spawn *protocol.Cell
}

// 20 values are reserved for tile types
// 0 means empty tile
type match struct {
	Players map[*LobbyMember]matchPlayer

	Board protocol.Board
}

func (m *match) MovePlayer(lobbyMember *LobbyMember, direction *protocol.Cell) bool {
	if direction.GetX() >= 0 && direction.GetX() < m.Board.GetHeight() && direction.GetY() >= 0 && direction.GetY() < m.Board.GetWidth() {
		if m.Board.Cells[direction.GetY() * m.Board.GetWidth() + direction.GetX()] == protocol.Terrain_TERRAIN_FLOOR {
			player := m.Players[lobbyMember]
			player.Spawn = direction
			return true
		}
	}
	return false
}

// TODO: implement bomb placement
func (m *match) PlayerPlaceBomb(lobbyMember *LobbyMember, newPos *protocol.Cell) {
}

type Match interface {
	MovePlayer(lobbyMember *LobbyMember, newPos *protocol.Cell) bool
	PlayerPlaceBomb(lobbyMember *LobbyMember, newPos *protocol.Cell)
}

func CreateMatch(lobbyMembers iter.Seq[*LobbyMember]) Match {
	spawnPoints := []*protocol.Cell{
		&protocol.Cell{X: 1, Y: 1},
		&protocol.Cell{X: 13, Y: 1},
		&protocol.Cell{X: 1, Y: 11},
		&protocol.Cell{X: 13, Y: 11},
	}
	rand.Shuffle(len(spawnPoints), func(i, j int) {
		spawnPoints[i], spawnPoints[j] = spawnPoints[j], spawnPoints[i]
	})

	matchInstance := &match{}

	i := 0
	for v := range lobbyMembers {
		matchInstance.Players[v] = matchPlayer{
			Spawn: spawnPoints[i],
		}
		i += 1
	}

	matchInstance.Board.Height = 13
	matchInstance.Board.Width = 15
	matchInstance.Board.Cells = []protocol.Terrain {
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
