package netio

import (
	"encoding/binary"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
	"io"
	"net"
	"iter"
	"errors"
)

func serialize(envelope *protocol.Envelope) ([]byte, error) {
	bytes, err := proto.Marshal(envelope)
	if err != nil {
		return nil, err
	}

	headBuf := make([]byte, 4)
	binary.BigEndian.PutUint32(headBuf, uint32(len(bytes)))

	packet := append(headBuf, bytes...)
	return packet, nil
}

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

	return proto.Unmarshal(dataBuf, envelope)
}

func SendMessage(conn net.Conn, envelope *protocol.Envelope) error {
	msg, err := serialize(envelope)
	if err != nil {
		return err
	}

	_, err = conn.Write(msg)
	return err
}

func BroadcastMessage(player_conns iter.Seq[net.Conn], envelope *protocol.Envelope) error {
	msg, err := serialize(envelope)
	if err != nil {
		return err
	}

	var errs []error
	for conn := range player_conns {
		_, err := conn.Write(msg)
		if err != nil {
			errs = append(errs, err)
		}
	}

	return errors.Join(errs...)
}

