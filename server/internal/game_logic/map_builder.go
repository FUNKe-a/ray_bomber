package gamelogic

import "github.com/FUNKe-a/ray_bomber/server/internal/gen/protocol"

type GameMap struct {
	Board  *protocol.Board
	Spawns []*protocol.Cell
}

type MapBuilder interface {
	Reset()
	BuildTerrain()
	BuildSpawns()
	GetMap() *GameMap
}

type MapDirector struct {
	builder MapBuilder
}

func NewMapDirector(builder MapBuilder) *MapDirector {
	return &MapDirector{builder: builder}
}

func (d *MapDirector) Construct() *GameMap {
	d.builder.Reset()
	d.builder.BuildTerrain()
	d.builder.BuildSpawns()

	return d.builder.GetMap()
}

var MapName = "classic"

func NewMapBuilder(name string) MapBuilder {
	if name == "open" {
		return &OpenMapBuilder{}
	}

	return &ClassicMapBuilder{}
}

var classicLayout = []string{
	"###############",
	"#.............#",
	"#.##.##.##.##.#",
	"#..C...C...C..#",
	"#.##.##.##.##.#",
	"#..C...C...C..#",
	"#.#.###.###.#.#",
	"#..C...C...C..#",
	"#.##.##.##.##.#",
	"#..C...C...C..#",
	"#.##.##.##.##.#",
	"#.............#",
	"###############",
}

var openLayout = []string{
	"###############",
	"#.............#",
	"#.............#",
	"#....C...C....#",
	"#.............#",
	"#..C...#...C..#",
	"#......#......#",
	"#..C...#...C..#",
	"#.............#",
	"#....C...C....#",
	"#.............#",
	"#.............#",
	"###############",
}

var terrainBySymbol = map[rune]protocol.Terrain{
	'#': protocol.Terrain_TERRAIN_WALL,
	'.': protocol.Terrain_TERRAIN_FLOOR,
	'C': protocol.Terrain_TERRAIN_CRATE,
}

func boardFromLayout(layout []string) *protocol.Board {
	board := &protocol.Board{
		Width:  uint32(len(layout[0])),
		Height: uint32(len(layout)),
	}

	for _, row := range layout {
		for _, symbol := range row {
			board.Cells = append(board.Cells, terrainBySymbol[symbol])
		}
	}

	return board
}

type ClassicMapBuilder struct {
	result GameMap
}

func (b *ClassicMapBuilder) Reset() {
	b.result = GameMap{}
}

func (b *ClassicMapBuilder) BuildTerrain() {
	b.result.Board = boardFromLayout(classicLayout)
}

func (b *ClassicMapBuilder) BuildSpawns() {
	b.result.Spawns = []*protocol.Cell{
		{X: 1, Y: 1},
		{X: 13, Y: 1},
		{X: 1, Y: 11},
		{X: 13, Y: 11},
	}
}

func (b *ClassicMapBuilder) GetMap() *GameMap {
	built := b.result
	return &built
}

type OpenMapBuilder struct {
	result GameMap
}

func (b *OpenMapBuilder) Reset() {
	b.result = GameMap{}
}

func (b *OpenMapBuilder) BuildTerrain() {
	b.result.Board = boardFromLayout(openLayout)
}

func (b *OpenMapBuilder) BuildSpawns() {
	b.result.Spawns = []*protocol.Cell{
		{X: 2, Y: 2},
		{X: 12, Y: 2},
		{X: 2, Y: 10},
		{X: 12, Y: 10},
	}
}

func (b *OpenMapBuilder) GetMap() *GameMap {
	built := b.result
	return &built
}
