package main

//go:generate go run scripts/protobuf.go

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"github.com/FUNKe-a/ray_bomber/server/internal/handlers"
	"log/slog"
	"net"
	"os"
)

type MsgWrapper struct {
	Conn net.Conn
	Msg  *protocol.Envelope
}

func main() {
	setupLogLevel()

	match := gamelogic.CreateMatch(15, 13)

	var idCounter uint32 = 20

	ln, _ := net.Listen("tcp", "127.0.0.1:6769")
	slog.Info("TCP socket opened on 127.0.0.1:6769")

	h_channel := make(chan MsgWrapper)

	go messageHandler(&match, h_channel)

	for {
		conn, _ := ln.Accept()

		go func(player_conn net.Conn, handler_c chan<- MsgWrapper) {
			for {
				var msg protocol.Envelope
				if err := netio.GetMessage(player_conn, &msg); err != nil {
					slog.Debug("Failed to receive message", "err", err)
				}

				handler_c <- MsgWrapper{Conn: player_conn, Msg: &msg}

			}
		}(conn, h_channel)

		idCounter += 1
	}
}

func messageHandler(match *gamelogic.GameMatch, c <-chan MsgWrapper) {
	for wrapper := range c {

		switch msg := wrapper.Msg.Payload.(type) {
		case *protocol.Envelope_MoveRequest:
			handlers.HandleMoveRequest(match, wrapper.Conn, msg.MoveRequest)
		case *protocol.Envelope_JoinLobbyRequest:
			handlers.HandleJoinLobbyRequest(match, wrapper.Conn, msg.JoinLobbyRequest)
		}
	}
}


func setupLogLevel() {
	opts := &slog.HandlerOptions{
		Level: slog.LevelDebug,
	}

	handler := slog.NewTextHandler(os.Stdout, opts)
	logger := slog.New(handler)

	slog.SetDefault(logger)
}
