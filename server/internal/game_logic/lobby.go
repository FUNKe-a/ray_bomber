package gamelogic

import (
	"errors"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"net"
	"maps"
)

type MatchPhase uint8

const (
	PhaseLobby MatchPhase = iota
	PhasePreparing
	PhasePlaying
)
const (
	tilePrefix uint32 = 19
)

var availableColors = []protocol.Color{
	protocol.Color_RED,
	protocol.Color_GREEN,
	protocol.Color_BLUE,
	protocol.Color_YELLOW,
}

type lobby struct {
	Members map[net.Conn]*protocol.PlayerInfo

	leaderId uint32 // Zero means no leader.
	Phase    MatchPhase
	matchCounter uint32
}

var lobbyInstance *lobby

func GetLobby() *lobby {
	if lobbyInstance == nil {
		lobbyInstance = new(lobby)
	}

	return lobbyInstance
}

func (lob *lobby) LeaderId() uint32 {
	return lob.leaderId
}

func (lob *lobby) AddMember(p_conn net.Conn, username string) (uint32, protocol.Color, error) {
	if player, exists := lob.Members[p_conn]; exists {
		return player.GetId(), player.GetColor(), errors.New("LobbyMember has already been added to lob.")
	}

	for _, color := range availableColors {
		isTaken := false
		for _, player := range lob.Members {
			if player.GetColor() == color {
				isTaken = true
				break
			}
		}

		if !isTaken {
			playerID := tilePrefix + uint32(color)
			lob.Members[p_conn] = &protocol.PlayerInfo{
				Id:       playerID,
				Username: username,
				Color:    color,
				IsReady:  false,
			}

			lob.updateLeader()

			return playerID, color, nil
		}
	}

	return 0, 0, nil
}

func (lob *lobby) RemoveMember(conn net.Conn) (*protocol.PlayerInfo, bool) {
	player, exists := lob.Members[conn]
	if !exists || player == nil {
		return nil, false
	}

	isLeader := player.GetId() == lob.leaderId

	delete(lob.Members, conn)

	if len(lob.Members) == 0 {
		lob.leaderId = 0
		lob.Phase = PhaseLobby
		return player, isLeader
	}

	if isLeader {
		lob.leaderId = 0
		lob.updateLeader()
	}

	return player, isLeader
}

func (lob *lobby) IsFull() bool {
	if len(lob.Members) >= 4 {
		return true
	}
	return false
}

func (lob *lobby) updateLeader() {
	if lob.leaderId == 0 && len(lob.Members) > 0 {
		for _, p := range lob.Members {
			lob.leaderId = p.GetId()
			return
		}
	}
}

func (lob *lobby) StartMatch() *Match {
	match := NewMatch(maps.Keys(lob.Members))
	match.id = lob.matchCounter
	lob.matchCounter += 1

	return match
}
