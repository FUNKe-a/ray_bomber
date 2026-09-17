package serdes

import (
	"errors"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
)

func Deserialize(messageType uint8, body []byte) (protocol.Message, error) {
	switch messageType {
	case protocol.MsgGreeting:
		return protocol.Message{Type: messageType, Data: protocol.Greeting{ID: body[0]}}, nil
	default:
		return protocol.Message{}, errors.New("unknown message type")
	}
}
