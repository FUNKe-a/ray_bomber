package main

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/serdes"
	"log/slog"
	"net"
)

type TraceableMessage struct {
	Conn net.Conn
	Msg  protocol.Message
}

func messageHandler(match *gamelogic.GameMatch, c <-chan TraceableMessage) {
	for bundle := range c {
		switch msg := bundle.Msg.Data.(type) {
		case protocol.MoveRequested:
			newX := match.Players[bundle.Conn].X
			newY := match.Players[bundle.Conn].Y

			switch msg.Direction {
			case 0:
				newY += 1
			case 1:
				newX += 1
			case 2:
				newY -= 1
			case 3:
				newX -= 1
			}

			if newX > 0 && newX < uint8(len(match.Board)) && newY > 0 && newY < uint8(len(match.Board[0])) {
				if match.Board[newX][newY] == gamelogic.EmptyTile {
					sendMsg := protocol.Message{Type: 0, Data: protocol.PlayerMoved{ID: match.Players[bundle.Conn].ID, X: newX, Y: newY}}
					serialized, _ := serdes.Serialize(sendMsg)
					match.Broadcast(serialized)
				}
			}
		}
	}
}

func main() {
	match := gamelogic.CreateMatch(15, 13)

	var idCounter uint8 = 20

	ln, _ := net.Listen("tcp", "127.0.0.1:6769")
	slog.Info("TCP socket opened on 127.0.0.1:6769")

	h_channel := make(chan TraceableMessage)

	for {
		conn, _ := ln.Accept()

		// TODO fix problem that idCounter will overfill
		// if people will join and leave
		match.Players[conn] = &gamelogic.Player{ID: idCounter}
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
				slog.Info("Message received.", "msgType", msg.Type)

			}
		}(conn, h_channel)

		idCounter += 1
	}
}
