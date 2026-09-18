package protocol

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
)

type handlerFunc func(PlayerID int, match *gamelogic.GameMatch) Message

type HandleableMessage interface {
	Handle(PlayerID int, grid [][]uint8) Message
}

func (msg *MoveRequested) Handle(PlayerID int, match *gamelogic.GameMatch) Message {
	return Message{Data: nil}
}

type MessageHandler interface {

}
