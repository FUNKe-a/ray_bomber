package protocol

const (
	MsgGreeting = iota
	MsgPlayerJoined
	MsgPlayerLeft
	MsgMoveRequested
	MsgPlayerMoved
)

type Message struct {
	Type uint8
	Data any
}

type Greeting struct {
	ID uint8
}

type PlayerJoined struct {
	ID     uint8
	StartX uint8
	StartY uint8
}

type PlayerLeft struct {
	ID uint8
}

type MoveRequested struct {
	Direction uint8
}

type PlayerMoved struct {
	ID uint8
	X  uint8
	Y  uint8
}
