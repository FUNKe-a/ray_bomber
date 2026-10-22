package handlers

import (
	"github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"github.com/FUNKe-a/ray_bomber/server/internal/net_io"
	"log/slog"
)

var lobby = gamelogic.GetLobby()

func TCPHandleMessages(messages <-chan netio.MsgWrapper) {
	var match_h matchHandler

	for wrapper := range messages {
		var err error

		if wrapper.Disconnected {
			err = handlePlayerDisconnect(wrapper.Conn)
		} else {
			switch msg := wrapper.Msg.Payload.(type) {
			case *protocol.Envelope_MoveRequest:
				match_h.handleMoveRequest(
					wrapper.Conn, msg.MoveRequest,
				)

			case *protocol.Envelope_JoinLobbyRequest:
				handleJoinLobbyRequest(
					wrapper.Conn, msg.JoinLobbyRequest,
				)

			case *protocol.Envelope_UpdateReadyState:
				handleUpdateReadyState(
					wrapper.Conn, msg.UpdateReadyState,
				)

			case *protocol.Envelope_GameStartRequest:
				if match, err := handleGameStartRequest(
					wrapper.Conn,
				); err == nil {
					match_h.Match = match
					match_h.sendMatchSetup()
				}

			case *protocol.Envelope_ClientMatchReady:
				match_h.handleClientMatchReady(
					wrapper.Conn,
				)
			}
		}

		if err != nil {
			slog.Warn("Handler failed", "err", err)
		}
	}
}
