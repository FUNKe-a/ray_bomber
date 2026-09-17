package gamelogic

import "net"

const (
	emptyTile = iota
	wallTile
)

type Player struct {
	ID   uint8
	Conn net.Conn
	X, Y int
}

// 0 means empty tile
type GameMatch struct {
	Players map[uint8]*Player
	Board   [15][13]uint8
	IdCount uint8
}

func CreateMatch() GameMatch {
	return GameMatch{Players: make(map[uint8]*Player), Board: [15][13]uint8{}, IdCount: 20}
}

func (gm *GameMatch) ValidateMove(playerID uint8, direction uint8) (x, y uint8) {
	return 0, 0
}
