package main

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/serdes"
	"log/slog"
	"net"
	"os"
)

func main() {
	setupLogLevel()

	match := gamelogic.CreateMatch(15, 13)

	var idCounter uint8 = 20

	ln, _ := net.Listen("tcp", "127.0.0.1:6769")
	slog.Info("TCP socket opened on 127.0.0.1:6769")

	h_channel := make(chan TraceableMessage)

	go messageHandler(&match, h_channel)

	for {
		conn, _ := ln.Accept()

		// TODO fix problem that idCounter will overfill
		// if people will join and leave
		match.Players[conn] = &gamelogic.Player{ID: idCounter, X: 1, Y: 1}
		greetMsg := protocol.Message{Type: 0, Data: protocol.Greeting{ID: idCounter}}
		greetInBytes, _ := serdes.Serialize(greetMsg)
		conn.Write(greetInBytes)

		go func(player_conn net.Conn, handler_c chan<- TraceableMessage) {
			for {
				head, body, err := netio.ReadMessage(player_conn)
				if err != nil {
					continue
				}

				msg, err := serdes.Deserialize(head, body)
				if err != nil {
					continue
				}
				slog.Debug("Message received.", "msgType", msg.Type)

				handler_c <- TraceableMessage{Conn: player_conn, Msg: msg}

			}
		}(conn, h_channel)

		idCounter += 1
	}
}

type TraceableMessage struct {
	Conn net.Conn
	Msg  protocol.Message
}

func setupLogLevel() {
	opts := &slog.HandlerOptions{
		Level: slog.LevelDebug,
	}

	handler := slog.NewTextHandler(os.Stdout, opts)
	logger := slog.New(handler)

	slog.SetDefault(logger)
}

func messageHandler(match *gamelogic.GameMatch, c <-chan TraceableMessage) {
	for bundle := range c {
		switch msg := bundle.Msg.Data.(type) {
		case protocol.MoveRequested:
			nX := int(match.Players[bundle.Conn].X)
			nY := int(match.Players[bundle.Conn].Y)

			switch msg.Direction {
			case 0:
				nY -= 1
			case 1:
				nY += 1
			case 2:
				nX -= 1
			case 3:
				nX += 1
			}

			if nY >= 0 && nY < len(match.Board) && nX >= 0 && nX < len(match.Board[0]) {
				if match.Board[nY][nX] == gamelogic.EmptyTile {
					match.Players[bundle.Conn].X = uint8(nX)
					match.Players[bundle.Conn].Y = uint8(nY)

					sendMsg := protocol.Message{Type: protocol.MsgPlayerMoved, Data: protocol.PlayerMoved{ID: match.Players[bundle.Conn].ID, X: uint8(nX), Y: uint8(nY)}}
					serialized, _ := serdes.Serialize(sendMsg)
					match.Broadcast(serialized)
					slog.Debug("Message broadcasted.", "msgType", sendMsg.Type)
				}
			}
		}
	}
}
