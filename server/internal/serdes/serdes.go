package serdes

import (
	"errors"
	"reflect"
	"bytes"
	"encoding/binary"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
)

func Deserialize(msgType uint8, body []byte) (protocol.Message, error) {
	switch msgType {
	case protocol.MsgGreeting:
		return protocol.Message{Type: msgType, Data: protocol.Greeting{ID: body[0]}}, nil
	case protocol.MsgMoveRequested:
		return protocol.Message{Type: msgType, Data: protocol.MoveRequested{Direction: body[0]}}, nil
	default:
		return protocol.Message{}, errors.New("unknown message type")
	}
}

func Serialize(msg protocol.Message) ([]byte, error) {
	if msg.Data == nil {
		return nil, errors.New("Message has an empty body")
	}

	buf := &bytes.Buffer{}

	binary.Write(buf, binary.BigEndian, msg.Type)
	binary.Write(buf, binary.BigEndian, uint16(reflect.TypeOf(msg.Data).Size()))
	binary.Write(buf, binary.BigEndian, msg.Data)

	return buf.Bytes(), nil
}
