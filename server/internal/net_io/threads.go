package netio

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"log/slog"
	"net"
)

type MsgWrapper struct {
	Conn         net.Conn
	Msg          *protocol.Envelope
	Disconnected bool
}

func ReadMessages(conn net.Conn, messages chan<- MsgWrapper) {
	defer func() {
		conn.Close()
		messages <- MsgWrapper{Conn: conn, Disconnected: true}
	}()

	for {
		var envelope protocol.Envelope

		if err := GetMessage(conn, &envelope); err != nil {
			slog.Debug("Connection ended", "err", err)
			return
		}

		messages <- MsgWrapper{Conn: conn, Msg: &envelope}
	}
}
