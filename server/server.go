package main

//go:generate go run scripts/protobuf.go

import (
	"log/slog"
	"net"
	"os"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/handlers"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

type MsgWrapper struct {
	Conn         net.Conn
	Msg          *protocol.Envelope
	Disconnected bool
}

func main() {
	setupLogLevel()

	match := gamelogic.CreateMatch(15, 13)

	ln, err := net.Listen("tcp", "127.0.0.1:6769")
	if err != nil {
		slog.Error("Failed to listen", "err", err)
		return
	}
	defer ln.Close()

	slog.Info("TCP socket opened on 127.0.0.1:6769")

	messages := make(chan MsgWrapper)
	go messageHandler(&match, messages)

	for {
		conn, err := ln.Accept()
		if err != nil {
			slog.Error("Failed to accept connection", "err", err)
			continue
		}

		slog.Debug("Client connected", "address", conn.RemoteAddr().String())
		go readMessages(conn, messages)
	}
}

func readMessages(conn net.Conn, messages chan<- MsgWrapper) {
	defer func() {
		conn.Close()
		messages <- MsgWrapper{Conn: conn, Disconnected: true}
	}()

	for {
		var envelope protocol.Envelope

		if err := netio.GetMessage(conn, &envelope); err != nil {
			slog.Debug("Connection ended", "err", err)
			return
		}

		messages <- MsgWrapper{Conn: conn, Msg: &envelope}
	}
}

func messageHandler(match *gamelogic.GameMatch, messages <-chan MsgWrapper) {
	for wrapper := range messages {
		var err error

		if wrapper.Disconnected {
			err = handlers.HandlePlayerDisconnect(match, wrapper.Conn)
		} else {
			switch msg := wrapper.Msg.Payload.(type) {
			case *protocol.Envelope_MoveRequest:
				err = handlers.HandleMoveRequest(
					match, wrapper.Conn, msg.MoveRequest,
				)

			case *protocol.Envelope_JoinLobbyRequest:
				err = handlers.HandleJoinLobbyRequest(
					match, wrapper.Conn, msg.JoinLobbyRequest,
				)

			case *protocol.Envelope_UpdateReadyState:
				err = handlers.HandleUpdateReadyState(
					match, wrapper.Conn, msg.UpdateReadyState,
				)

			case *protocol.Envelope_GameStartRequest:
				err = handlers.HandleGameStartRequest(
					match, wrapper.Conn, msg.GameStartRequest,
				)

			case *protocol.Envelope_ClientMatchReady:
				err = handlers.HandleClientMatchReady(
					match, wrapper.Conn, msg.ClientMatchReady,
				)
			}
		}

		if err != nil {
			slog.Warn("Handler failed", "err", err)
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
