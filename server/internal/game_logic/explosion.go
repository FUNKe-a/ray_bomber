package gamelogic

import "github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"

type Position struct {
	X int32
	Y int32
}

type ExplosionBoard interface {
	TerrainAt(Position) protocol.Terrain
	HasBomb(Position) bool
}

type ExplosionStrategy interface {
	Calculate(
		board ExplosionBoard,
		origin Position,
		blastRange uint32,
	) []Position
}

type CrossBlastStrategy struct{}

func (CrossBlastStrategy) Calculate(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
) []Position {
	cells := []Position{origin}

	directions := []Position{
		{X: 0, Y: -1},
		{X: 1, Y: 0},
		{X: 0, Y: 1},
		{X: -1, Y: 0},
	}

	for _, direction := range directions {
		for distance := uint32(1); distance <= blastRange; distance++ {
			cell := Position{
				X: origin.X + direction.X*int32(distance),
				Y: origin.Y + direction.Y*int32(distance),
			}

			terrain := board.TerrainAt(cell)

			if terrain == protocol.Terrain_TERRAIN_WALL {
				break
			}

			cells = append(cells, cell)

			if terrain == protocol.Terrain_TERRAIN_CRATE ||
				board.HasBomb(cell) {
				break
			}
		}
	}

	return cells
}

// Read-only view used for all explosions within one simulation tick.
type explosionView struct {
	board *protocol.Board
	bombs map[Position]uint64
}

func (view explosionView) TerrainAt(
	position Position,
) protocol.Terrain {
	width := int32(view.board.GetWidth())
	height := int32(view.board.GetHeight())

	if position.X < 0 || position.X >= width ||
		position.Y < 0 || position.Y >= height {
		return protocol.Terrain_TERRAIN_WALL
	}

	index := int(position.Y)*int(width) + int(position.X)

	return view.board.Cells[index]
}

func (view explosionView) HasBomb(position Position) bool {
	_, exists := view.bombs[position]
	return exists
}
