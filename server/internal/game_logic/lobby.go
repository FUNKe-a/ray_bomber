package gamelogic

import (
	"errors"
	"maps"
	"net"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
)

type MatchPhase uint8

const (
	PhaseLobby MatchPhase = iota
	PhasePreparing
	PhasePlaying
	PhaseFinished
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

	leaderId     uint32 // Zero means no leader.
	Phase        MatchPhase
	matchCounter uint64
	joinOrder    []net.Conn
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
	if lob.Members == nil {
		lob.Members = make(map[net.Conn]*protocol.PlayerInfo)
	}

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

			lob.joinOrder = append(lob.joinOrder, p_conn)
			lob.updateLeader()

			return playerID, color, nil
		}
	}

	return 0, protocol.Color_UNKNOWN, errors.New("lobby is full")
}

func (lob *lobby) RemoveMember(conn net.Conn) (*protocol.PlayerInfo, bool) {
	player, exists := lob.Members[conn]
	if !exists || player == nil {
		return nil, false
	}

	isLeader := player.GetId() == lob.leaderId

	delete(lob.Members, conn)

	for i, memberConn := range lob.joinOrder {
		if memberConn == conn {
			lob.joinOrder = append(
				lob.joinOrder[:i],
				lob.joinOrder[i+1:]...,
			)
			break
		}
	}

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
	if lob.leaderId != 0 {
		return
	}

	for _, conn := range lob.joinOrder {
		if member, exists := lob.Members[conn]; exists {
			lob.leaderId = member.GetId()
			return
		}
	}
}

func (lob *lobby) StartMatch() *Match {
	lob.matchCounter++

	gameMap := NewMapDirector(NewMapBuilder(MapName)).Construct()

	match := NewMatch(maps.Keys(lob.Members), gameMap)
	match.id = lob.matchCounter

	for conn, member := range lob.Members {
		match.Players[conn].ID = member.GetId()
	}

	return match
}
