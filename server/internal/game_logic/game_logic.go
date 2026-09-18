package gamelogic

import "net"

const (
	EmptyTile = iota
	WallTile
)

type Player struct {
	ID   uint8
	X, Y uint8
}

// 20 values are reserved for tile types
// 0 means empty tile
type GameMatch struct {
	Players map[net.Conn]*Player
	Board   [][]uint8
	IdCount uint8
}

func CreateMatch(sizeX uint8, sizeY uint8) GameMatch {
	// board := make([][]uint8, sizeY)
	// for i := range board {
	// 	board[i] = make([]uint8, sizeX)
	// }

	board := [13][15]uint8 {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1,},
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

	return GameMatch{Players: make(map[net.Conn]*Player), Board: slice, IdCount: 20}
}

func (board *GameMatch) Broadcast(data []byte) {
	for k := range board.Players {
		k.Write(data)
	}
}
