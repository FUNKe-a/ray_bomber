package handlers

import (
	"errors"
	"log/slog"
	"time"

	gamelogic "github.com/FUNKe-a/ray_bomber/server/internal/game_logic"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	netio "github.com/FUNKe-a/ray_bomber/server/internal/net_io"
)

var lobby = gamelogic.GetLobby()

func TCPHandleMessages(messages <-chan netio.MsgWrapper) {
	var matchH matchHandler

	var preparationTimer *time.Timer
	var preparationTimeout <-chan time.Time

	stopPreparationTimer := func() {
		if preparationTimer != nil {
			preparationTimer.Stop()
		}

		preparationTimer = nil
		preparationTimeout = nil
	}

	defer stopPreparationTimer()

	for {
		select {
		case <-preparationTimeout:
			stopPreparationTimer()

			if lobby.Phase == gamelogic.PhasePreparing {
				err := matchH.cancelPreparation(
					protocol.LobbyResetReason_RESET_PREPARATION_TIMEOUT,
				)
				if err != nil {
					slog.Warn("Preparation reset failed", "err", err)
				}
			}

		case wrapper, open := <-messages:
			if !open {
				return
			}

			var err error
			if wrapper.Disconnected {
				cancelPreparation :=
					matchH.Match != nil &&
						lobby.Phase == gamelogic.PhasePreparing &&
						matchH.Match.PlayerExists(wrapper.Conn)

				if matchH.Match != nil {
					delete(matchH.Match.Players, wrapper.Conn)
				}

				err = handlePlayerDisconnect(wrapper.Conn)
				if cancelPreparation {
					stopPreparationTimer()

					err = errors.Join(
						err,
						matchH.cancelPreparation(
							protocol.LobbyResetReason_RESET_PREPARATION_DISCONNECT,
						),
					)
				}
			} else if wrapper.Msg != nil {
				switch msg := wrapper.Msg.Payload.(type) {
				case *protocol.Envelope_MoveRequest:
					err = matchH.handleMoveRequest(
						wrapper.Conn, msg.MoveRequest,
					)

				case *protocol.Envelope_JoinLobbyRequest:
					err = handleJoinLobbyRequest(
						wrapper.Conn, msg.JoinLobbyRequest,
					)

				case *protocol.Envelope_UpdateReadyState:
					err = handleUpdateReadyState(
						wrapper.Conn, msg.UpdateReadyState,
					)

				case *protocol.Envelope_GameStartRequest:
					nextMatch, startErr :=
						handleGameStartRequest(wrapper.Conn)

					err = startErr
					if nextMatch != nil {
						matchH.Match = nextMatch

						// Retain the match even if a broadcast failed.
						// Closed connections are removed by their
						// queued disconnect events.
						err = errors.Join(
							err, matchH.sendMatchSetup(),
						)

						stopPreparationTimer()
						preparationTimer = time.NewTimer(15 * time.Second)
						preparationTimeout = preparationTimer.C
					}

				case *protocol.Envelope_ClientMatchReady:
					err = matchH.handleClientMatchReady(
						wrapper.Conn,
						msg.ClientMatchReady,
					)

					if lobby.Phase == gamelogic.PhasePlaying {
						stopPreparationTimer()
					}
				}
			}

			if err != nil {
				slog.Warn("Handler failed", "err", err)
			}
		}
	}
}
