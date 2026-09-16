package main

import (
	"encoding/binary"
	"fmt"
	"github.com/FUNKe-a/ray_bomber/server/internal/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/serdes"
	"io"
	"net"
)

func main() {
	ln, err := net.Listen("tcp", "127.0.0.1:6769")
	if err != nil {
		// handle error
	}
	for {
		c, err := ln.Accept()
		if err != nil {
			// handle error
		}

		go func(conn net.Conn) {
			conn.Write([]byte{0x00, 0x00, 0x01, 0x01})

			headBuffer := make([]byte, 3)
			for {
				io.ReadFull(conn, headBuffer)

				msgLength := binary.BigEndian.Uint16(headBuffer[1:3])

				bodyBuffer := make([]byte, msgLength)
				io.ReadFull(conn, bodyBuffer)
				msg, err := serdes.Deserialize(headBuffer[0], bodyBuffer)

				switch msg.(type) {
				case protocol.MoveRequested:
					fmt.Println("requested move")
				}

				if err != nil {
					break
				}
			}
		}(c)
	}
}
