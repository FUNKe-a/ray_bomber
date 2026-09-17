package main

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"fmt"
	"net"
)

// const (
// 	emptyTile = iota
// 	wallTile
// 	bombTile
// )
//
// type Player struct {
// 	ID   uint8
// 	Conn net.Conn
// 	X, Y int
// }

func main() {
	// Players := make(map[uint8]*Player)
	// gameBoard := [15][13]*uint8{}
	// IdCounter := 10
	ln, _ := net.Listen("tcp", "127.0.0.1:6769")

	h_channel := make(chan protocol.Message)

	for {
		conn, _ := ln.Accept()

		go func(player_conn net.Conn, handler_c chan protocol.Message) {
			player_conn.Write([]byte{0x00, 0x00, 0x01, 0x01})

			for {
				_, err := netio.ReadAndDeserialize(player_conn)
				if err != nil {
					fmt.Println(err)
					continue 
				}
			}
		}(conn, h_channel)
	}
}
