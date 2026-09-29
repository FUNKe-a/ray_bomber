package main

//go:generate protoc --proto_path=../proto --go_out=internal/gen/protocol --go_opt=paths=source_relative ../proto/player.proto ../proto/envelope.proto

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"log/slog"
	"net"
	"os"
)

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

		// TODO fix problem that idCounter will overfill
		// if people will join and leave
		match.Players[conn] = &gamelogic.Player{ID: idCounter, X: 1, Y: 1}
		msg := &protocol.Envelope{
			Payload: &protocol.Envelope_Greeting{
				Greeting: &protocol.Greeting{
					Id: idCounter,
				},
			},
		}
		netio.SendMessage(conn, msg)

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

type MsgWrapper struct {
	Conn net.Conn
	Msg  *protocol.Envelope
}

func setupLogLevel() {
	opts := &slog.HandlerOptions{
		Level: slog.LevelDebug,
	}

	handler := slog.NewTextHandler(os.Stdout, opts)
	logger := slog.New(handler)

	slog.SetDefault(logger)
}

func messageHandler(match *gamelogic.GameMatch, c <-chan MsgWrapper) {
	for wrapper := range c {
		conn := wrapper.Conn
		player := match.Players[conn]

		switch msg := wrapper.Msg.Payload.(type) {
		case *protocol.Envelope_MoveRequest:
			new_x := player.X
			new_y := player.Y

			switch msg.MoveRequest.Direction {
			case protocol.MoveRequest_UP:
				new_y -= 1
			case protocol.MoveRequest_RIGHT:
				new_x += 1
			case protocol.MoveRequest_DOWN:
				new_y += 1
			case protocol.MoveRequest_LEFT:
				new_x -= 1
			}

			if new_x >= 0 && new_x < int32(len(match.Board[0])) && new_y >= 0 && new_y < int32(len(match.Board)) {
				if match.Board[new_y][new_x] == gamelogic.EmptyTile {
					msg := &protocol.Envelope{
						Payload: &protocol.Envelope_PlayerMovement{
							PlayerMovement: &protocol.PlayerMovement{
								Id: player.ID,
								X:  new_x,
								Y:  new_y,
							},
						},
					}

					netio.SendMessage(conn, msg)
				}
			}
		}
	}
}
