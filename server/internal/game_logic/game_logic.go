package gamelogic

import (
	"errors"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"net"
)

const (
	EmptyTile = iota
	WallTile
)

const (
	tilePrefix uint32 = 19
)

var availableColors = []protocol.Color{
	protocol.Color_RED,
	protocol.Color_GREEN,
	protocol.Color_BLUE,
	protocol.Color_YELLOW,
}

type Player struct {
	ID         uint32
	Username   string
	X, Y       int32
	Color      protocol.Color
	IsReady    bool
	MatchReady bool
}

type MatchPhase uint8

const (
	PhaseLobby MatchPhase = iota
	PhasePreparing
	PhasePlaying
)

// 20 values are reserved for tile types
// 0 means empty tile
type GameMatch struct {
	Players map[net.Conn]*Player
	Board   [][]uint8
	IDCount uint8

	LeaderID uint32 // Zero means no leader.
	Phase    MatchPhase
}

func (match *GameMatch) AddPlayer(p_conn net.Conn, username string) (uint32, protocol.Color, error) {
	if player, exists := match.Players[p_conn]; exists {
		return player.ID, player.Color, errors.New("Player has already been added to match.")
	}

	for _, color := range availableColors {
		isTaken := false
		for _, player := range match.Players {
			if player.Color == color {
				isTaken = true
				break
			}
		}

		if !isTaken {
			var spawnX, spawnY int32

			switch color {
			case protocol.Color_RED:
				spawnX, spawnY = 1, 1
			case protocol.Color_GREEN:
				spawnX, spawnY = 13, 1
			case protocol.Color_BLUE:
				spawnX, spawnY = 1, 11
			case protocol.Color_YELLOW:
				spawnX, spawnY = 13, 11
			}

			playerID := tilePrefix + uint32(color)
			match.Players[p_conn] = &Player{
				ID:       playerID,
				Username: username,
				X:        spawnX,
				Y:        spawnY,
				Color:    color,
			}

			match.updateLeader()

			return playerID, color, nil
		}
	}

	return 0, 0, nil
}

func (match *GameMatch) RemovePlayer(conn net.Conn) (*Player, bool) {
	player := match.Players[conn]

	delete(match.Players, conn)

	isLeader := player.ID == match.LeaderID

	if isLeader {
		match.LeaderID = 0
		match.updateLeader()
	}

	return player, isLeader
}

func (match *GameMatch) IsFull() bool {
	if len(match.Players) >= 4 {
		return true
	}
	return false
}

func (match *GameMatch) updateLeader() {
	if match.LeaderID == 0 {
		for _, p := range match.Players {
			match.LeaderID = p.ID
			return
		}
	}
}

func CreateMatch(sizeX uint8, sizeY uint8) GameMatch {
	// board := make([][]uint8, sizeY)
	// for i := range board {
	// 	board[i] = make([]uint8, sizeX)
	// }

	board := [13][15]uint8{
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	}

	slice := make([][]uint8, len(board))
	for i := range board {
		slice[i] = board[i][:]
	}

	return GameMatch{Players: make(map[net.Conn]*Player), Board: slice, IDCount: 20}
}
