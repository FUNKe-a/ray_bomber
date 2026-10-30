package netio

import (
	"encoding/binary"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/serdes"
	"io"
	"net"
)

func ReadAndDeserialize(conn net.Conn) (protocol.Message, error) {
	if msg_type, msg_body, err := ReadMessage(conn); err == nil {
		return serdes.Deserialize(msg_type, msg_body)
	}

	return protocol.Message{}, nil
}

func ReadMessage(conn net.Conn) (uint8, []byte, error) {
	headBuffer := make([]byte, 3)
	if _, err := io.ReadFull(conn, headBuffer); err != nil {
		return 0, nil, err
	}

	msgType := headBuffer[0]
	msgLength := binary.BigEndian.Uint16(headBuffer[1:3])

	bodyBuffer := make([]byte, msgLength)
	if _, err := io.ReadFull(conn, bodyBuffer); err != nil {
		return 0, nil, err
	}

	return msgType, bodyBuffer, nil
}
