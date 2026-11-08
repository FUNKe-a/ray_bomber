package gamelogic

import (
	"fmt"
	"log/slog"

	"github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"
	"google.golang.org/protobuf/proto"
)

type MapPrototype interface {
	Clone() *GameMap
}

var _ MapPrototype = (*GameMap)(nil)

func (m *GameMap) Clone() *GameMap {
	return m.DeepClone()
}

// Shares the board and spawns with the prototype.
func (m *GameMap) ShallowClone() *GameMap {
	clone := *m
	return &clone
}

func (m *GameMap) DeepClone() *GameMap {
	spawns := make([]*protocol.Cell, len(m.Spawns))

	for i, spawn := range m.Spawns {
		spawns[i] = &protocol.Cell{
			X: spawn.GetX(),
			Y: spawn.GetY(),
		}
	}

	return &GameMap{
		Board:  proto.Clone(m.Board).(*protocol.Board),
		Spawns: spawns,
	}
}

type MapRegistry struct {
	prototypes map[string]*GameMap
}

func NewMapRegistry(names ...string) *MapRegistry {
	registry := &MapRegistry{
		prototypes: make(map[string]*GameMap),
	}

	for _, name := range names {
		registry.prototypes[name] =
			NewMapDirector(NewMapBuilder(name)).Construct()
	}

	return registry
}

func (r *MapRegistry) Get(name string) *GameMap {
	prototype, exists := r.prototypes[name]
	if !exists {
		prototype = r.prototypes["classic"]
	}

	clone := prototype.Clone()

	logMapAddresses("Map prototype", prototype)
	logMapAddresses("Map clone", clone)

	return clone
}

var mapRegistry = NewMapRegistry("classic", "open")

func logMapAddresses(label string, m *GameMap) {
	slog.Debug(
		label,
		"map", fmt.Sprintf("%p", m),
		"board", fmt.Sprintf("%p", m.Board),
		"cells", fmt.Sprintf("%p", m.Board.Cells),
		"spawns", fmt.Sprintf("%p", m.Spawns),
		"firstSpawn", fmt.Sprintf("%p", m.Spawns[0]),
	)
}
