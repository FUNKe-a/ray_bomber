package main

//go:generate go run scripts/protobuf.go

import (
	"log/slog"
	"net"
	"os"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/handlers"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)


func main() {
	setupLogLevel()

	if name := os.Getenv("RAY_BOMBER_MAP"); name != "" {
		gamelogic.MapName = name
	}
	slog.Info("Map layout", "name", gamelogic.MapName)

	ln, err := net.Listen("tcp", "127.0.0.1:6769")
	if err != nil {
		slog.Error("Failed to listen", "err", err)
		return
	}
	defer ln.Close()

	slog.Info("TCP socket opened on 127.0.0.1:6769")

	messages := make(chan netio.MsgWrapper, 30)
	go handlers.TCPHandleMessages(messages)

	for {
		conn, err := ln.Accept()
		if err != nil {
			slog.Error("Failed to accept connection", "err", err)
			continue
		}

		slog.Debug("Client connected", "address", conn.RemoteAddr().String())
		go netio.ReadMessages(conn, messages)
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
