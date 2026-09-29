package netio

import (
	"encoding/binary"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
	"io"
	"net"
)

func GetMessage(conn net.Conn, envelope *protocol.Envelope) error {
	headBuf := make([]byte, 4)

	if _, err := io.ReadFull(conn, headBuf); err != nil {
		return err
	}

	length := binary.BigEndian.Uint32(headBuf)

	dataBuf := make([]byte, length)
	if _, err := io.ReadFull(conn, dataBuf); err != nil {
		return err
	}

	var msg protocol.Envelope;
	err := proto.Unmarshal(dataBuf, &msg)

	return err
}

func SendMessage(conn net.Conn, envelope *protocol.Envelope) error {
	bytes, err := proto.Marshal(envelope)
	if err != nil {
		return err
	}

	headBuf := make([]byte, 4)
	binary.BigEndian.PutUint32(headBuf, uint32(len(bytes)))

	packet := append(headBuf, bytes...)

	_, err = conn.Write(packet)
	return err
}
