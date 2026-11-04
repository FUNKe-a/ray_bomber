package netio

import (
	"encoding/binary"
	"errors"
	"io"
	"iter"
	"net"
	"time"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
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
	packet, err := serialize(envelope)
	if err != nil {
		return err
	}

	return writePacket(conn, packet)
}

func BroadcastMessage(
	playerConns iter.Seq[net.Conn],
	envelope *protocol.Envelope,
) error {
	packet, err := serialize(envelope)
	if err != nil {
		return err
	}

	var errs []error

	for conn := range playerConns {
		if err := writePacket(conn, packet); err != nil {
			errs = append(errs, err)
		}
	}

	return errors.Join(errs...)
}

func writePacket(conn net.Conn, packet []byte) error {
	if err := conn.SetWriteDeadline(time.Now().Add(2 * time.Second)); err != nil {
		conn.Close()
		return err
	}

	defer func() {
		_ = conn.SetWriteDeadline(time.Time{})
	}()

	for len(packet) > 0 {
		n, err := conn.Write(packet)

		if err != nil {
			conn.Close()
			return err
		}

		if n == 0 {
			conn.Close()
			return io.ErrShortWrite
		}

		packet = packet[n:]
	}

	return nil
}
