package main

import (
	"fmt"
	// "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/serdes"
	"net"
)

func main() {
	// match := gamelogic.CreateMatch()

	ln, _ := net.Listen("tcp", "127.0.0.1:6769")

	h_channel := make(chan protocol.Message)

	for {
		conn, _ := ln.Accept()

		go func(player_conn net.Conn, handler_c chan protocol.Message) {

			for {
				head, body, err := netio.ReadMessage(player_conn)
				if err != nil {
					fmt.Println(err)
					continue
				}
				msg, err := serdes.Deserialize(head, body)
				if err != nil {
					fmt.Println(err)
					continue
				}

				msgInBytes, err := serdes.Serialize(msg)
				player_conn.Write(msgInBytes)
			}
		}(conn, h_channel)
	}
}
