package gamelogic

import (
	"errors"
	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"net"
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

type LobbyMember struct {
	Information *protocol.PlayerInfo

	MatchReady bool
}

type lobby struct {
	LobbyMembers map[net.Conn]*LobbyMember
	IDCount      uint8

	LeaderID uint32 // Zero means no leader.
	Phase    MatchPhase
}

var lobbyInstance *lobby

func GetLobby() *lobby {
	if lobbyInstance == nil {
		lobbyInstance = new(lobby)
	}

	return lobbyInstance
}

func (match *lobby) AddLobbyMember(p_conn net.Conn, username string) (uint32, protocol.Color, error) {
	if player, exists := match.LobbyMembers[p_conn]; exists {
		return player.Information.GetId(), player.Information.GetColor(), errors.New("LobbyMember has already been added to match.")
	}

	for _, color := range availableColors {
		isTaken := false
		for _, player := range match.LobbyMembers {
			if player.Information.GetColor() == color {
				isTaken = true
				break
			}
		}

		if !isTaken {
			playerID := tilePrefix + uint32(color)
			match.LobbyMembers[p_conn] = &LobbyMember{
				Information: &protocol.PlayerInfo{
					Id:       playerID,
					Username: username,
					Color:    color,
					IsReady:  false,
				},
			}

			match.updateLeader()

			return playerID, color, nil
		}
	}

	return 0, 0, nil
}

func (match *lobby) RemoveLobbyMember(conn net.Conn) (*LobbyMember, bool) {
	player, exists := match.LobbyMembers[conn]
	if !exists || player == nil {
		return nil, false
	}

	isLeader := player.Information.GetId() == match.LeaderID

	delete(match.LobbyMembers, conn)

	if len(match.LobbyMembers) == 0 {
		match.LeaderID = 0
		match.Phase = PhaseLobby
		return player, isLeader
	}

	if isLeader {
		match.LeaderID = 0
		match.updateLeader()
	}

	return player, isLeader
}

func (match *lobby) IsFull() bool {
	if len(match.LobbyMembers) >= 4 {
		return true
	}
	return false
}

func (match *lobby) updateLeader() {
	if match.LeaderID == 0 && len(match.LobbyMembers) > 0 {
		for _, p := range match.LobbyMembers {
			match.LeaderID = p.Information.GetId()
			return
		}
	}
}
