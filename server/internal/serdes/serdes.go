package serdes

import (
	"errors"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
)

func Deserialize(messageType byte, body []byte) (any, error) {
	switch messageType {
	case protocol.MsgGreeting:
		return protocol.Greeting{ID: body[0]}, nil
	case protocol.MsgMoveRequested:
		return protocol.MoveRequested{Direction: body[0]}, nil
	default:
		return nil, errors.New("unknown message type")
	}
}
