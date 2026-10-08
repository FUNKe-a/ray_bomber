package gamelogic

import (
	"log/slog"
	"net"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
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

	JoinOrder []net.Conn
	LeaderID  uint32 // Zero means no leader.
	Phase     MatchPhase
}

func (match *GameMatch) AddPlayer(p_conn net.Conn, username string) (uint32, protocol.Color) {
	if player, exists := match.Players[p_conn]; exists {
		slog.Debug("Player already added to match.")
		return player.ID, player.Color
	}

	for _, color := range availableColors {
		isTaken := false

		for _, player := range match.Players {
			if player.Color == color {
				isTaken = true
				break
			}
		}

		if isTaken == false {
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

			match.JoinOrder = append(match.JoinOrder, p_conn)

			if match.LeaderID == 0 {
				match.LeaderID = playerID
			}

			return playerID, color
		}
	}

	return 0, 0
}

func (match *GameMatch) RemovePlayer(conn net.Conn) (*Player, bool) {
	player, exists := match.Players[conn]
	if !exists {
		return nil, false
	}

	delete(match.Players, conn)

	for i, candidate := range match.JoinOrder {
		if candidate == conn {
			match.JoinOrder = append(
				match.JoinOrder[:i],
				match.JoinOrder[i+1:]...,
			)
			break
		}
	}

	previousLeader := match.LeaderID
	match.LeaderID = 0

	if len(match.JoinOrder) > 0 {
		match.LeaderID = match.Players[match.JoinOrder[0]].ID
	} else {
		match.Phase = PhaseLobby
	}

	return player, previousLeader != match.LeaderID
}

func (match *GameMatch) IsFull(p_conn net.Conn) bool {
	if len(match.Players) >= 4 {
		return true
	}
	return false
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
