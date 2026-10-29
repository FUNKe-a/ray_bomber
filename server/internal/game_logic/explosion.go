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
type DiagonalBlastStrategy struct{}
type HorizontalBlastStrategy struct{}
type VerticalBlastStrategy struct{}

// Compile-time checks that all four types implement the interface.
var (
	_ ExplosionStrategy = CrossBlastStrategy{}
	_ ExplosionStrategy = DiagonalBlastStrategy{}
	_ ExplosionStrategy = HorizontalBlastStrategy{}
	_ ExplosionStrategy = VerticalBlastStrategy{}
)

func (CrossBlastStrategy) Calculate(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
) []Position {
	return calculateRays(board, origin, blastRange, []Position{
		{X: 0, Y: -1},
		{X: 1, Y: 0},
		{X: 0, Y: 1},
		{X: -1, Y: 0},
	})
}

func (DiagonalBlastStrategy) Calculate(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
) []Position {
	return calculateRays(board, origin, blastRange, []Position{
		{X: -1, Y: -1},
		{X: 1, Y: -1},
		{X: 1, Y: 1},
		{X: -1, Y: 1},
	})
}

func (HorizontalBlastStrategy) Calculate(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
) []Position {
	return calculateRays(board, origin, blastRange, []Position{
		{X: -1, Y: 0},
		{X: 1, Y: 0},
	})
}

func (VerticalBlastStrategy) Calculate(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
) []Position {
	return calculateRays(board, origin, blastRange, []Position{
		{X: 0, Y: -1},
		{X: 0, Y: 1},
	})
}

// Each direction is a unit step along one explosion ray.
// The origin is assumed to be a valid bomb position.
func calculateRays(
	board ExplosionBoard,
	origin Position,
	blastRange uint32,
	directions []Position,
) []Position {
	cells := []Position{origin}

	for _, direction := range directions {
		current := origin

		for step := uint32(0); step < blastRange; step++ {
			// A diagonal ray cannot pass around a wall corner.
			if direction.X != 0 && direction.Y != 0 {
				horizontal := Position{
					X: current.X + direction.X,
					Y: current.Y,
				}
				vertical := Position{
					X: current.X,
					Y: current.Y + direction.Y,
				}

				if board.TerrainAt(horizontal) == protocol.Terrain_TERRAIN_WALL ||
					board.TerrainAt(vertical) == protocol.Terrain_TERRAIN_WALL {
					break
				}
			}

			next := Position{
				X: current.X + direction.X,
				Y: current.Y + direction.Y,
			}

			terrain := board.TerrainAt(next)

			// Walls and board boundaries are excluded.
			if terrain == protocol.Terrain_TERRAIN_WALL {
				break
			}

			cells = append(cells, next)

			// Crates and bombs are included, but stop this ray.
			if terrain == protocol.Terrain_TERRAIN_CRATE ||
				board.HasBomb(next) {
				break
			}

			current = next
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
